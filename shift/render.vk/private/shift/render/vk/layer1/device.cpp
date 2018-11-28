#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/layer1/physical_device.hpp"
#include "shift/render/vk/layer1/instance.hpp"
#include "shift/render/vk/window_glfw.hpp"
#include "shift/render/vk/renderer_impl.hpp"
#include "shift/render/vk/utility.hpp"
#include "shift/render/vk/resource_streamer.hpp"
#include <shift/resource_db/repository.hpp>
#include <shift/resource_db/image.hpp>
#include <shift/resource_db/buffer_view.hpp>
#include "shift/task/async.hpp"
#include <shift/core/exception.hpp>
#include <shift/log/log.hpp>
#include <exception>
#include <cstdint>

namespace shift::render::vk::layer1
{
thread_local std::unique_ptr<device::worker_thread_local_data>
  device::_worker_local;

template <typename T>
void initialize_pools()
{
  object_pool<shared_wrapper<T>>::initialize_pool();
  object_pool<framed_shared_wrapper<T>>::initialize_pool();
  object_pool<fenced_shared_wrapper<T>>::initialize_pool();
}

template <typename T>
void finalize_pools()
{
  object_pool<fenced_shared_wrapper<T>>::finalize_pool();
  object_pool<framed_shared_wrapper<T>>::finalize_pool();
  object_pool<shared_wrapper<T>>::finalize_pool();
}

device::worker_thread_local_data::worker_thread_local_data(
  vk::layer1::device& device)
: transient_graphics_command_pool(
    device, device.graphics_queue_family_index(),
    vk::command_pool_create_flag::transient_bit |
      vk::command_pool_create_flag::reset_command_buffer_bit),
  durable_graphics_command_pool(device, device.graphics_queue_family_index(),
                                vk::command_pool_create_flag::none),
  transient_compute_command_pool(
    device, device.compute_queue_family_index(),
    vk::command_pool_create_flag::transient_bit |
      vk::command_pool_create_flag::reset_command_buffer_bit),
  durable_compute_command_pool(device, device.compute_queue_family_index(),
                               vk::command_pool_create_flag::none),
  transient_transfer_command_pool(
    device, device.transfer_queue_family_index(),
    vk::command_pool_create_flag::transient_bit |
      vk::command_pool_create_flag::reset_command_buffer_bit),
  durable_transfer_command_pool(device, device.transfer_queue_family_index(),
                                vk::command_pool_create_flag::none)
{
  shared_object_queue::initialize();
  framed_object_queue::initialize();
  fenced_object_queue::initialize();
  initialize_pools<vk::layer1::buffer>();
  initialize_pools<vk::layer1::image>();
  initialize_pools<vk::layer1::image_view>();
  initialize_pools<vk::layer1::sampler>();
  initialize_pools<vk::layer1::descriptor_pool>();
  initialize_pools<vk::layer1::descriptor_set>();
  initialize_pools<vk::layer1::pipeline>();
  initialize_pools<vk::layer1::framebuffer>();
  initialize_pools<vk::layer1::render_pass>();
  initialize_pools<vk::layer1::query_pool>();
  initialize_pools<vk::layer1::timestamp_query_pool>();
  initialize_pools<vk::layer1::command_buffer>();
}

device::worker_thread_local_data::~worker_thread_local_data()
{
  finalize_pools<vk::layer1::buffer>();
  finalize_pools<vk::layer1::image>();
  finalize_pools<vk::layer1::image_view>();
  finalize_pools<vk::layer1::sampler>();
  finalize_pools<vk::layer1::descriptor_pool>();
  finalize_pools<vk::layer1::descriptor_set>();
  finalize_pools<vk::layer1::pipeline>();
  finalize_pools<vk::layer1::framebuffer>();
  finalize_pools<vk::layer1::render_pass>();
  finalize_pools<vk::layer1::query_pool>();
  finalize_pools<vk::layer1::timestamp_query_pool>();
  finalize_pools<vk::layer1::command_buffer>();
  fenced_object_queue::finalize();
  framed_object_queue::finalize();
  shared_object_queue::finalize();
}

device::device(vk::layer1::instance& instance,
               const vk::layer1::physical_device& physical_device,
               vk::window_glfw* window)
: _physical_device(&physical_device), _memory_manager(*this, *_physical_device)
{
  if (window != nullptr)
  {
    BOOST_ASSERT(window->surface().physical_device().handle() ==
                 physical_device.handle());
    if (window->surface().physical_device().handle() !=
        physical_device.handle())
    {
      BOOST_THROW_EXCEPTION(
        shift::core::logic_error()
        << shift::core::context_info("A device created from a window must use "
                                     "the same physical device as the one "
                                     "backing up the window's surface."));
    }
  }

  bool khr_maintenance2_found = false;

  auto enabled_extensions = instance.enabled_extensions();
  std::uint32_t device_extension_count = 0;
  vk_check(vk::enumerate_device_extension_properties(
    physical_device.handle(), nullptr, &device_extension_count, nullptr));
  if (device_extension_count > 0)
  {
    std::vector<vk::extension_properties> device_extensions;
    device_extensions.resize(device_extension_count);
    vk_check(vk::enumerate_device_extension_properties(
      physical_device.handle(), nullptr, &device_extension_count,
      device_extensions.data()));
    for (const auto& instance_extension : device_extensions)
    {
      std::string extension_name = instance_extension.extension_name().data();
      if (extension_name == VK_KHR_MAINTENANCE2_EXTENSION_NAME)
      {
        khr_maintenance2_found = true;
        enabled_extensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
      }
    }
  }
  if (!khr_maintenance2_found)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Cannot find required Vulkan extension.")
      << shift::core::context_info(VK_KHR_MAINTENANCE2_EXTENSION_NAME));
  }

  // Search for a graphics queue that supports sparse binding, and a present
  // queue in the array of queue families.
  /// ToDo: The following code doesn't work reliable. Use a recursive
  /// backtracking algorithm instead!
  std::vector<vk::queue_family_properties> queue_family_properties;
  _physical_device->queue_family_properties(queue_family_properties);
  bool found_graphics_queue = false;
  bool found_compute_queue = false;
  bool found_transfer_queue = false;

  // Build list of available queues.
  std::vector<queue_properties> available_queues;
  for (std::uint32_t queue_family_index = 0u;
       queue_family_index < queue_family_properties.size();
       queue_family_index++)
  {
    VkBool32 queue_family_supports_presenting = VK_FALSE;
    if (window != nullptr)
    {
      instance.get_physical_device_surface_support_khr(
        _physical_device->handle(), queue_family_index,
        window->surface().handle(), &queue_family_supports_presenting);
    }
    available_queues.push_back(queue_properties{
      queue_family_index,
      queue_family_properties[queue_family_index].queue_flags(),
      queue_family_supports_presenting == VK_TRUE,
      queue_family_properties[queue_family_index].queue_count(), 0});
  }
  for (auto& available_queue : available_queues)
  {
    // Look for a dedicated graphics queue that also supports presentation
    // (unless there is no window).
    if (!found_graphics_queue && (available_queue.available > 0) &&
        (available_queue.flags & vk::queue_flag::graphics_bit) &&
        (available_queue.flags & vk::queue_flag::sparse_binding_bit) &&
        (window == nullptr || available_queue.supports_presenting))
    {
      _graphics_queue_family_index = available_queue.family_index;
      found_graphics_queue = true;
      --available_queue.available;
      ++available_queue.selected;
      continue;
    }

    // Look for a dedicated compute queue family.
    if (!found_compute_queue && (available_queue.available > 0) &&
        (available_queue.flags & vk::queue_flag::compute_bit))
    {
      _compute_queue_family_index = available_queue.family_index;
      found_compute_queue = true;
      --available_queue.available;
      ++available_queue.selected;
      continue;
    }

    // Look for a dedicated transfer queue family.
    if (!found_transfer_queue && (available_queue.available > 0) &&
        (available_queue.flags & vk::queue_flag::transfer_bit))
    {
      _transfer_queue_family_index = available_queue.family_index;
      found_transfer_queue = true;
      --available_queue.available;
      ++available_queue.selected;
      continue;
    }
  }

  if (!found_graphics_queue)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("The selected physical device does not "
                                   "offer any compatible graphics command "
                                   "queues.")
      << shift::core::context_info(
           _physical_device->properties().device_name().data()));
  }
  if (!found_compute_queue)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("The selected physical device does not "
                                   "offer any compatible compute command "
                                   "queues.")
      << shift::core::context_info(
           _physical_device->properties().device_name().data()));
  }
  if (!found_transfer_queue)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("The selected physical device does not "
                                   "offer any compatible transfer command "
                                   "queues.")
      << shift::core::context_info(
           _physical_device->properties().device_name().data()));
  }

  std::array<float, 4> queue_priorities = {{0.0f, 0.0f, 0.0f, 0.0f}};

  std::vector<vk::device_queue_create_info> queue_create_infos;
  for (const auto& available_queue : available_queues)
  {
    if (available_queue.selected > 0)
    {
      queue_create_infos.emplace_back(
        /* next */ nullptr,
        /* flags */ vk::device_queue_create_flag::none,
        /* queue_family_index */ available_queue.family_index,
        /* queue_count */ available_queue.selected,
        /* queue_priorities */ &queue_priorities[0]);
    }
  }

  /// ToDo: Don't use assertions here.
  const auto& multiview_features = _physical_device->multiview_features();
  BOOST_ASSERT(multiview_features.multiview());
  BOOST_ASSERT(multiview_features.multiview_geometry_shader());
  BOOST_ASSERT(multiview_features.multiview_tessellation_shader());

  vk_check(vk::create_device(
    _physical_device->handle(),
    core::rvalue_address(vk::device_create_info(
      /* next */ &multiview_features,
      /* flags */ vk::device_create_flag::none,
      /* queue_create_info_count */
      static_cast<std::uint32_t>(queue_create_infos.size()),
      /* queue_create_infos */ queue_create_infos.data(),
      /* enabled_layer_count */
      static_cast<std::uint32_t>(instance.enabled_layers().size()),
      /* enabled_layer_names */ instance.enabled_layers().data(),
      /* enabled_extension_count */
      static_cast<std::uint32_t>(enabled_extensions.size()),
      /* enabled_extension_names */ enabled_extensions.data(),
      /// ToDo: Select set of actually enabled features!
      /* enabled_features */ &physical_device.features())),
    renderer_impl::singleton_instance().default_allocator(), &_device));
  _graphics_queue.create(*this, _graphics_queue_family_index, 0);
  _compute_queue.create(*this, _compute_queue_family_index, 0);
  _transfer_queue.create(*this, _transfer_queue_family_index, 0);
  _graphics_queue_mutex_ptr = &_graphics_queue_mutex;
  _compute_queue_mutex_ptr =
    (_transfer_queue->handle() != _graphics_queue->handle())
      ? &_compute_queue_mutex
      : &_graphics_queue_mutex;
  _transfer_queue_mutex_ptr =
    (_transfer_queue->handle() != _graphics_queue->handle())
      ? (_transfer_queue->handle() != _compute_queue->handle())
          ? &_transfer_queue_mutex
          : &_compute_queue_mutex
      : &_graphics_queue_mutex;

  _create_swapchain_khr = reinterpret_cast<PFN_vkCreateSwapchainKHR>(
    proc_address("vkCreateSwapchainKHR"));
  _destroy_swapchain_khr = reinterpret_cast<PFN_vkDestroySwapchainKHR>(
    proc_address("vkDestroySwapchainKHR"));
  _get_swapchain_images_khr = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(
    proc_address("vkGetSwapchainImagesKHR"));
  _acquire_next_image_khr = reinterpret_cast<PFN_vkAcquireNextImageKHR>(
    proc_address("vkAcquireNextImageKHR"));
  _queue_present_khr =
    reinterpret_cast<PFN_vkQueuePresentKHR>(proc_address("vkQueuePresentKHR"));
}

device::~device()
{
  _memory_manager.clear();
  if (_device != nullptr)
  {
    _transfer_queue.reset();
    _compute_queue.reset();
    _graphics_queue.reset();
    vk::destroy_device(_device,
                       renderer_impl::singleton_instance().default_allocator());
    _device = nullptr;
  }
}

void device::create_worker_resources(task::worker_id_t /*worker_id*/)
{
  _worker_local = std::make_unique<worker_thread_local_data>(*this);
}

void device::destroy_worker_resources(task::worker_id_t /*worker_id*/)
{
  _worker_local.reset();
}

vk::layer1::command_pool& device::select_command_pool(queue_type type,
                                                      bool transient)
{
  auto& worker_local = *_worker_local;
  switch (type)
  {
  case queue_type::graphics:
    return transient ? worker_local.transient_graphics_command_pool
                     : worker_local.durable_graphics_command_pool;

  case queue_type::compute:
    return transient ? worker_local.transient_compute_command_pool
                     : worker_local.durable_compute_command_pool;

  case queue_type::transfer:
    return transient ? worker_local.transient_transfer_command_pool
                     : worker_local.durable_transfer_command_pool;
  }

  BOOST_ASSERT(false);
  std::terminate();
}

// vk::shared_ptr<vk::layer1::command_buffer> device::create_command_buffer(
//  queue_type type, bool transient, vk::command_buffer_level level)
//{
//  auto& worker_local = *_worker_local;
//  switch (type)
//  {
//  case queue_type::graphics:
//    return vk::layer1::command_buffer::create(
//      transient ? worker_local.transient_graphics_command_pool
//                : worker_local.durable_graphics_command_pool,
//      level);
//  case queue_type::compute:
//    return vk::layer1::command_buffer::create(
//      transient ? worker_local.transient_compute_command_pool
//                : worker_local.durable_compute_command_pool,
//      level);
//  case queue_type::transfer:
//    return vk::layer1::command_buffer::create(
//      transient ? worker_local.transient_transfer_command_pool
//                : worker_local.durable_transfer_command_pool,
//      level);
//  }
//  BOOST_ASSERT(false);
//  return nullptr;
//}

// vk::shared_ptr<vk::layer1::buffer> device::create_buffer(
//  const vk::buffer_create_info& create_info)
//{
//  return vk::layer1::buffer::create(*this, create_info);
//}

// vk::shared_ptr<vk::layer1::image> device::create_image(
//  const image_create_info& create_info)
//{
//  return vk::layer1::image::create(*this, create_info);
//}

// vk::shared_ptr<vk::layer1::image> device::create_image(
//  VkImage image_handle, vk::image_type type, vk::format format,
//  vk::extent_3d extent, std::uint32_t mip_levels, std::uint32_t array_layers)
//{
//  return vk::layer1::image::create(image_handle, type, format, extent,
//                                   mip_levels, array_layers);
//}

// vk::shared_ptr<vk::layer1::image_view> device::create_image_view(
//  const image_view_create_info& create_info, vk::layer1::image& image)
//{
//  return vk::layer1::image_view::create(*this, create_info, image);
//}

// vk::shared_ptr<vk::layer1::sampler> device::create_sampler(
//  const sampler_create_info& create_info)
//{
//  return vk::layer1::sampler::create(*this, create_info);
//}

// vk::shared_ptr<vk::layer1::descriptor_pool> device::create_descriptor_pool(
//  const vk::descriptor_pool_create_info& descriptor_pool_create_info)
//{
//  return vk::layer1::descriptor_pool::create(*this,
//                                             descriptor_pool_create_info);
//}

// vk::shared_ptr<vk::layer1::descriptor_set> device::create_descriptor_set(
//  vk::layer1::descriptor_pool& descriptor_pool,
//  vk::layer1::descriptor_set_layout& descriptor_set_layout)
//{
//  return vk::layer1::descriptor_set::create(*this, descriptor_pool,
//                                            descriptor_set_layout);
//}

// vk::shared_ptr<vk::layer1::pipeline> device::create_pipeline(
//  vk::layer1::pipeline_cache& pipeline_cache,
//  const graphics_pipeline_create_info& create_info)
//{
//  return vk::layer1::pipeline::create(*this, pipeline_cache, create_info);
//}

// vk::shared_ptr<vk::layer1::pipeline> device::create_pipeline(
//  vk::layer1::pipeline_cache& pipeline_cache,
//  const compute_pipeline_create_info& create_info)
//{
//  return vk::layer1::pipeline::create(*this, pipeline_cache, create_info);
//}

// vk::shared_ptr<vk::layer1::framebuffer> device::create_framebuffer(
//  const framebuffer_create_info& create_info)
//{
//  return vk::layer1::framebuffer::create(*this, create_info);
//}

// vk::shared_ptr<vk::layer1::render_pass> device::create_render_pass(
//  const render_pass_create_info& create_info)
//{
//  return vk::layer1::render_pass::create(*this, create_info);
//}

// vk::shared_ptr<vk::layer1::query_pool> device::create_query_pool(
//  const query_pool_create_info& create_info)
//{
//  return vk::layer1::query_pool::create(*this, create_info);
//}

// vk::shared_ptr<vk::layer1::timestamp_query_pool>
// device::create_timestamp_query_pool(const query_pool_create_info&
// create_info)
//{
//  return vk::layer1::timestamp_query_pool::create(*this, create_info);
//}

void device::update_descriptor_set(
  const vk::write_descriptor_set& write_descriptor_set)
{
  vk::update_descriptor_sets(_device, 1u, &write_descriptor_set, 0, nullptr);
}

vk::result device::wait(uint32_t fence_count, const VkFence* fence_handles,
                        bool wait_for_all, std::chrono::nanoseconds timeout)
{
  return vk::wait_for_fences(_device, fence_count, fence_handles,
                             wait_for_all ? VK_TRUE : VK_FALSE,
                             static_cast<std::uint64_t>(timeout.count()));
}

void device::wait_idle()
{
  vk::device_wait_idle(_device);
}

vk::result device::create_swapchain(
  const vk::swapchain_create_info_khr&& create_info,
  const vk::allocation_callbacks* allocator, VkSwapchainKHR* swapchain)
{
  return static_cast<vk::result>(_create_swapchain_khr(
    _device, reinterpret_cast<const VkSwapchainCreateInfoKHR*>(&create_info),
    reinterpret_cast<const VkAllocationCallbacks*>(allocator), swapchain));
}

void device::destroy_swapchain(VkSwapchainKHR swapchain,
                               const vk::allocation_callbacks* allocator)
{
  return _destroy_swapchain_khr(
    _device, swapchain,
    reinterpret_cast<const VkAllocationCallbacks*>(allocator));
}

vk::result device::swapchain_images(VkSwapchainKHR swapchain,
                                    std::uint32_t* swapchain_image_count,
                                    VkImage* swapchain_images)
{
  return static_cast<vk::result>(_get_swapchain_images_khr(
    _device, swapchain, swapchain_image_count, swapchain_images));
}

vk::result device::acquire_next_image(VkSwapchainKHR swapchain,
                                      std::uint64_t timeout,
                                      VkSemaphore semaphore, VkFence fence,
                                      std::uint32_t& image_index)
{
  return static_cast<vk::result>(_acquire_next_image_khr(
    _device, swapchain, timeout, semaphore, fence, &image_index));
}

vk::result device::queue_present(VkQueue queue,
                                 const vk::present_info_khr* present_info)
{
  return static_cast<vk::result>(_queue_present_khr(
    queue, reinterpret_cast<const VkPresentInfoKHR*>(present_info)));
}

PFN_vkVoidFunction device::proc_address(const char* name)
{
  if (auto result = vk::get_device_proc_addr(_device, name))
    return result;
  else
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Unable to find function address.")
      << shift::core::context_info(name));
  }
}
}
