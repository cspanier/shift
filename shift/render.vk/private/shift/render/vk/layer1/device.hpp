#ifndef SHIFT_RENDER_VK_LAYER1_DEVICE_HPP
#define SHIFT_RENDER_VK_LAYER1_DEVICE_HPP

#include <memory>
#include <vector>
#include <chrono>
#include <mutex>
#include <shift/task/types.hpp>
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/layer1/buffer.hpp"
#include "shift/render/vk/layer1/image.hpp"
#include "shift/render/vk/layer1/image_view.hpp"
#include "shift/render/vk/layer1/sampler.hpp"
#include "shift/render/vk/layer1/surface.hpp"
#include "shift/render/vk/layer1/swapchain.hpp"
#include "shift/render/vk/layer1/descriptor_set.hpp"
#include "shift/render/vk/layer1/pipeline.hpp"
#include "shift/render/vk/layer1/framebuffer.hpp"
#include "shift/render/vk/layer1/render_pass.hpp"
#include "shift/render/vk/layer1/query_pool.hpp"
#include "shift/render/vk/layer1/timestamp_query_pool.hpp"
#include "shift/render/vk/layer1/command_queue.hpp"
#include "shift/render/vk/layer1/command_pool.hpp"
#include "shift/render/vk/layer1/command_buffer.hpp"
#include "shift/render/vk/memory_manager.hpp"
#include "shift/render/vk/window.hpp"

namespace shift::render::vk
{
class window_glfw;

///
class locked_command_queue
{
public:
  locked_command_queue() = delete;

  ///
  locked_command_queue(vk::layer1::command_queue& queue, std::mutex& mutex)
  : _queue(&queue), _lock(mutex)
  {
  }

  locked_command_queue(const locked_command_queue&) = delete;
  locked_command_queue(locked_command_queue&&) = default;
  ~locked_command_queue() = default;
  locked_command_queue& operator=(const locked_command_queue&) = delete;
  locked_command_queue& operator=(locked_command_queue&&) = default;

  vk::layer1::command_queue* operator->()
  {
    return _queue;
  }

  const vk::layer1::command_queue* operator->() const
  {
    return _queue;
  }

private:
  vk::layer1::command_queue* _queue;
  std::unique_lock<std::mutex> _lock;
};
}

namespace shift::render::vk::layer1
{
class instance;
class physical_device;

enum queue_type
{
  graphics,
  compute,
  transfer
};

///
/// @remarks
///   The type is not moveable because several other types hold a pointer to
///   their associated device instance.
class device
{
public:
  /// Constructor.
  device(vk::layer1::instance& instance,
         const vk::layer1::physical_device& physical_device,
         vk::window_glfw* window);

  device(const device&) = delete;
  device(device&&) = delete;

  /// Destructor.
  ~device();

  device& operator=(const device&) = delete;
  device& operator=(device&&) = delete;

  ///
  VkDevice& handle() noexcept
  {
    return _device;
  }

  ///
  const VkDevice& handle() const noexcept
  {
    return _device;
  }

  ///
  const vk::layer1::physical_device& physical_device() const
  {
    return *_physical_device;
  }

  ///
  std::uint32_t graphics_queue_family_index() const
  {
    return _graphics_queue_family_index;
  }

  ///
  std::uint32_t compute_queue_family_index() const
  {
    return _compute_queue_family_index;
  }

  ///
  std::uint32_t transfer_queue_family_index() const
  {
    return _transfer_queue_family_index;
  }

  ///
  vk::locked_command_queue graphics_queue()
  {
    return vk::locked_command_queue(*_graphics_queue,
                                    *_graphics_queue_mutex_ptr);
  }

  ///
  vk::locked_command_queue compute_queue()
  {
    return vk::locked_command_queue(*_compute_queue, *_compute_queue_mutex_ptr);
  }

  ///
  vk::locked_command_queue transfer_queue()
  {
    return vk::locked_command_queue(*_transfer_queue,
                                    *_transfer_queue_mutex_ptr);
  }

  ///
  vk::memory_manager& memory_manager()
  {
    return _memory_manager;
  }

  ///
  void create_worker_resources(task::worker_id_t worker_id);

  ///
  void destroy_worker_resources(task::worker_id_t worker_id);

  ///
  vk::layer1::command_pool& select_command_pool(queue_type type,
                                                bool transient);

  //  ///
  //  vk::shared_ptr<vk::layer1::command_buffer> create_command_buffer(
  //    queue_type type, bool transient, vk::command_buffer_level level);

  //  ///
  //  vk::shared_ptr<vk::layer1::buffer> create_buffer(
  //    const vk::buffer_create_info& create_info);

  //  ///
  //  vk::shared_ptr<vk::layer1::image> create_image(
  //    const image_create_info& create_info);

  //  ///
  //  vk::shared_ptr<vk::layer1::image> create_image(
  //    VkImage image_handle, vk::image_type type, vk::format format,
  //    vk::extent_3d extent, std::uint32_t mip_levels, std::uint32_t
  //    array_layers);

  //  ///
  //  vk::shared_ptr<vk::layer1::image_view> create_image_view(
  //    const image_view_create_info& create_info, vk::layer1::image& image);

  //  ///
  //  vk::shared_ptr<vk::layer1::sampler> create_sampler(
  //    const sampler_create_info& create_info);

  //  ///
  //  vk::shared_ptr<vk::layer1::descriptor_pool> create_descriptor_pool(
  //    const vk::descriptor_pool_create_info& descriptor_pool_create_info);

  //  ///
  //  vk::shared_ptr<vk::layer1::descriptor_set> create_descriptor_set(
  //    vk::layer1::descriptor_pool& descriptor_pool,
  //    vk::layer1::descriptor_set_layout& descriptor_set_layout);

  //  ///
  //  vk::shared_ptr<vk::layer1::pipeline> create_pipeline(
  //    vk::layer1::pipeline_cache& pipeline_cache,
  //    const graphics_pipeline_create_info& create_info);

  //  ///
  //  vk::shared_ptr<vk::layer1::pipeline> create_pipeline(
  //    vk::layer1::pipeline_cache& pipeline_cache,
  //    const compute_pipeline_create_info& create_info);

  //  ///
  //  vk::shared_ptr<vk::layer1::framebuffer> create_framebuffer(
  //    const framebuffer_create_info& create_info);

  //  ///
  //  vk::shared_ptr<vk::layer1::render_pass> create_render_pass(
  //    const render_pass_create_info& create_info);

  //  ///
  //  vk::shared_ptr<vk::layer1::query_pool> create_query_pool(
  //    const query_pool_create_info& create_info);

  //  ///
  //  vk::shared_ptr<vk::layer1::timestamp_query_pool>
  //  create_timestamp_query_pool(
  //    const query_pool_create_info& create_info);

  ///
  void update_descriptor_set(
    const vk::write_descriptor_set& write_descriptor_set);

  ///
  template <std::size_t Count>
  void update_descriptor_sets(
    const std::array<vk::write_descriptor_set, Count>& write_descriptor_sets)
  {
    vk::update_descriptor_sets(
      _device, static_cast<std::uint32_t>(write_descriptor_sets.size()),
      write_descriptor_sets.data(), 0, nullptr);
  }

  ///
  vk::result wait(uint32_t fence_count, const VkFence* fence_handles,
                  bool wait_for_all, std::chrono::nanoseconds timeout);

  ///
  vk::result wait(const vk::layer1::fence& fence,
                  std::chrono::nanoseconds timeout)
  {
    return wait(1u, &fence.handle(), true, timeout);
  }

  ///
  template <std::size_t FenceCount>
  bool wait(const std::array<VkFence, FenceCount>& fence_handles,
            bool wait_for_all, std::chrono::nanoseconds timeout)
  {
    return wait(static_cast<std::uint32_t>(fence_handles.size()),
                fence_handles.data(), wait_for_all, timeout);
  }

  ///
  template <std::size_t FenceCount>
  bool wait(const std::array<vk::layer1::fence, FenceCount>& fences,
            bool wait_for_all, std::chrono::nanoseconds timeout)
  {
    std::array<VkFence, FenceCount> fence_handles;
    for (std::size_t i = 0; i < FenceCount; ++i)
      fence_handles[i] = fences[i].handle();
    return wait(static_cast<std::uint32_t>(fence_handles.size()),
                fence_handles.data(), wait_for_all, timeout);
  }

  ///
  void wait_idle();

private:
  friend class swapchain;
  friend class command_queue;

  ///
  vk::result create_swapchain(const vk::swapchain_create_info_khr&& create_info,
                              const vk::allocation_callbacks* allocator,
                              VkSwapchainKHR* swapchain);

  ///
  void destroy_swapchain(VkSwapchainKHR swapchain,
                         const vk::allocation_callbacks* allocator);

  ///
  vk::result swapchain_images(VkSwapchainKHR swapchain,
                              std::uint32_t* swapchain_image_count,
                              VkImage* swapchain_images);

  ///
  vk::result acquire_next_image(VkSwapchainKHR swapchain, std::uint64_t timeout,
                                VkSemaphore semaphore, VkFence fence,
                                std::uint32_t& image_index);

  ///
  vk::result queue_present(VkQueue queue,
                           const vk::present_info_khr* present_info);

private:
  ///
  PFN_vkVoidFunction proc_address(const char* name);

private:
  ///
  struct queue_properties
  {
    std::uint32_t family_index;
    vk::queue_flags flags;
    bool supports_presenting;
    std::uint32_t available;
    std::uint32_t selected;
  };

  ///
  struct worker_thread_local_data
  {
    ///
    worker_thread_local_data(vk::layer1::device& device);

    ///
    ~worker_thread_local_data();

    vk::layer1::command_pool transient_graphics_command_pool;
    vk::layer1::command_pool durable_graphics_command_pool;
    vk::layer1::command_pool transient_compute_command_pool;
    vk::layer1::command_pool durable_compute_command_pool;
    vk::layer1::command_pool transient_transfer_command_pool;
    vk::layer1::command_pool durable_transfer_command_pool;
  };

  const vk::layer1::physical_device* _physical_device = nullptr;

  VkDevice _device = nullptr;
  std::uint32_t _graphics_queue_family_index = 0;
  std::uint32_t _compute_queue_family_index = 0;
  std::uint32_t _transfer_queue_family_index = 0;
  core::stack_ptr<vk::layer1::command_queue> _graphics_queue;
  core::stack_ptr<vk::layer1::command_queue> _compute_queue;
  core::stack_ptr<vk::layer1::command_queue> _transfer_queue;
  std::mutex _graphics_queue_mutex;
  std::mutex _compute_queue_mutex;
  std::mutex _transfer_queue_mutex;
  std::mutex* _graphics_queue_mutex_ptr = nullptr;
  std::mutex* _compute_queue_mutex_ptr = nullptr;
  std::mutex* _transfer_queue_mutex_ptr = nullptr;

  PFN_vkCreateSwapchainKHR _create_swapchain_khr = nullptr;
  PFN_vkDestroySwapchainKHR _destroy_swapchain_khr = nullptr;
  PFN_vkGetSwapchainImagesKHR _get_swapchain_images_khr = nullptr;
  PFN_vkAcquireNextImageKHR _acquire_next_image_khr = nullptr;
  PFN_vkQueuePresentKHR _queue_present_khr = nullptr;

  vk::memory_manager _memory_manager;
  /// ToDo: thread_local static prevents parallel instantiation of multiple
  /// device instances.
  thread_local static std::unique_ptr<worker_thread_local_data> _worker_local;
};
}

#endif
