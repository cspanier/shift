#include "shift/render/vk/layer1/instance.hpp"
#include "shift/render/vk/renderer_impl.hpp"
#include "shift/render/vk/application.hpp"
#include <shift/log/log.hpp>
#include <shift/core/exception.hpp>
#include <shift/platform/environment.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <array>
#include <cstdint>

namespace shift::render::vk::layer1
{
instance::instance(const vk::application_info& application_info,
                   vk::debug_layers debug_layers, vk::debug_logs debug_logs,
                   const vk::allocation_callbacks* allocator)
: _allocator(allocator)
{
  // Check supported Vulkan version.
  std::uint32_t api_version = VK_API_VERSION_1_0;
  // The driver only supports Vulkan 1.0 if there is no
  // vkEnumerateInstanceVersion.
  if (auto* enumerate_instance_version =
        reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
          vk::get_instance_proc_addr(nullptr, "vkEnumerateInstanceVersion"));
      enumerate_instance_version != nullptr)
  {
    vk_check(static_cast<vk::result>(enumerate_instance_version(&api_version)));
  }
  log::info() << "Driver supports Vulkan version "
              << VK_VERSION_MAJOR(api_version) << "."
              << VK_VERSION_MINOR(api_version) << ".";
  if (application_info.api_version() > api_version)
  {
    BOOST_THROW_EXCEPTION(shift::core::runtime_error()
                          << shift::core::context_info(
                               "The supported version of Vulkan is too low."));
  }

  // Specify debug layers.
  if (debug_layers & vk::debug_layer::standard_validation)
    _enabled_layers.push_back("VK_LAYER_LUNARG_standard_validation");
  if (debug_layers & vk::debug_layer::monitor)
    _enabled_layers.push_back("VK_LAYER_LUNARG_monitor");
  if (debug_layers & vk::debug_layer::screenshot)
    _enabled_layers.push_back("VK_LAYER_LUNARG_screenshot");
  if (debug_layers & vk::debug_layer::api_dump)
    _enabled_layers.push_back("VK_LAYER_LUNARG_api_dump");
  // if (debug_layers & vk::debug_layer::renderdoc_capture)
  //  _enabled_layers.push_back("VK_LAYER_RENDERDOC_Capture");
  // if (debug_layers & vk::debug_layer::steam_overlay)
  //  _enabled_layers.push_back("VK_LAYER_VALVE_steam_overlay");
  // if (debug_layers & vk::debug_layer::optimus)
  //  _enabled_layers.push_back("VK_LAYER_NV_optimus");

  std::uint32_t existing_instance_layer_count = 0;
  std::vector<vk::layer_properties> existing_instance_layers;
  vk_check(vk::enumerate_instance_layer_properties(
    &existing_instance_layer_count, nullptr));
  if (existing_instance_layer_count > 0)
  {
    existing_instance_layers.resize(existing_instance_layer_count);
    vk_check(vk::enumerate_instance_layer_properties(
      &existing_instance_layer_count, existing_instance_layers.data()));
  }
  for (const auto& enabled_layer : _enabled_layers)
  {
    bool found = false;
    for (const auto& existing_layer : existing_instance_layers)
    {
      if (std::strcmp(enabled_layer, existing_layer.layer_name().data()) == 0)
      {
        log::debug() << "Enabling " << existing_layer.description().data()
                     << " " << existing_layer.layer_name().data();
        found = true;
        break;
      }
    }
    if (!found)
    {
      BOOST_THROW_EXCEPTION(shift::core::runtime_error()
                            << shift::core::context_info(
                                 "Cannot find enabled Vulkan validation layer.")
                            << shift::core::context_info(enabled_layer));
    }
  }

  // Check if all required instance extensions are supported.
  std::array<const char*, 4> required_extensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(SHIFT_PLATFORM_WINDOWS)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(SHIFT_PLATFORM_LINUX)
#if defined(USE_WAYLAND)
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#else
    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
#endif
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    /// ToDo: The following extensions should have been promoted to Vulkan 1.1.
    /// We still need at least VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2 due to a
    /// crash on Linux with Nvidia driver 390.48. Is this a driver bug?
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
  };

  {
    std::vector<vk::extension_properties> instance_extensions;
    std::uint32_t instance_extension_count = 0;
    vk_check(vk::enumerate_instance_extension_properties(
      nullptr, &instance_extension_count, nullptr));
    if (instance_extension_count > 0)
    {
      instance_extensions.resize(instance_extension_count);
      vk_check(vk::enumerate_instance_extension_properties(
        nullptr, &instance_extension_count, instance_extensions.data()));
    }
    for (auto required_extension : required_extensions)
    {
      bool found_extension = false;
      for (const auto& instance_extension : instance_extensions)
      {
        if (std::string_view{required_extension} ==
            std::string_view{instance_extension.extension_name().data()})
        {
          found_extension = true;
          break;
        }
      }
      if (!found_extension)
      {
        BOOST_THROW_EXCEPTION(
          shift::core::runtime_error()
          << shift::core::context_info("Cannot find required Vulkan extension.")
          << shift::core::context_info(required_extension));
      }
    }
  }

  switch (vk::create_instance(
    core::rvalue_address(vk::instance_create_info(
      /* next */ nullptr,
      /* flags */ vk::instance_create_flag::none,
      /* application_info */ &application_info,
      /* enabled_layer_count */
      static_cast<std::uint32_t>(_enabled_layers.size()),
      /* enabled_layer_names */ _enabled_layers.empty()
        ? nullptr
        : _enabled_layers.data(),
      /* enabled_extension_count */
      static_cast<std::uint32_t>(required_extensions.size()),
      /* enabled_extension_names */ required_extensions.data())),
    _allocator, &_instance))
  {
  case vk::result::success:
    break;

  case vk::result::error_incompatible_driver:
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Cannot find a compatible Vulkan "
                                   "installable client driver (ICD).\n\nPlease "
                                   "look at the Getting Started guide for "
                                   "additional information.")
      << shift::core::context_info("vkCreateInstance"));

  case vk::result::error_extension_not_present:
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Cannot find a specified extension "
                                   "library.\nMake sure your layers path is "
                                   "set appropriately.")
      << shift::core::context_info("vkCreateInstance"));

  default:
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info(
           "vkCreateInstance failed.\n\nDo you have a compatible Vulkan "
           "installable client driver (ICD) installed?\nPlease look at the "
           "Getting Started guide for additional information.")
      << shift::core::context_info("vkCreateInstance"));
  }

  if (debug_logs)
  {
    _create_debug_report_callback_ext =
      reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
        proc_address("vkCreateDebugReportCallbackEXT"));
    _destroy_debug_report_callback_ext =
      reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
        proc_address("vkDestroyDebugReportCallbackEXT"));
    _debug_report_message_ext = reinterpret_cast<PFN_vkDebugReportMessageEXT>(
      proc_address("vkDebugReportMessageEXT"));

    static_assert(static_cast<std::uint32_t>(vk::debug_log::information) ==
                  static_cast<std::uint32_t>(
                    vk::debug_report_flag_ext::information_bit_ext));
    static_assert(
      static_cast<std::uint32_t>(vk::debug_log::warning) ==
      static_cast<std::uint32_t>(vk::debug_report_flag_ext::warning_bit_ext));
    static_assert(
      static_cast<std::uint32_t>(vk::debug_log::performance_warning) ==
      static_cast<std::uint32_t>(
        vk::debug_report_flag_ext::performance_warning_bit_ext));
    static_assert(
      static_cast<std::uint32_t>(vk::debug_log::error) ==
      static_cast<std::uint32_t>(vk::debug_report_flag_ext::error_bit_ext));
    static_assert(
      static_cast<std::uint32_t>(vk::debug_log::debug) ==
      static_cast<std::uint32_t>(vk::debug_report_flag_ext::debug_bit_ext));
    vk::debug_report_callback_create_info_ext debug_create_info(
      /* next */ nullptr,
      /* flags */ static_cast<vk::debug_report_flags_ext>(debug_logs.data()),
      /* pfn_callback */ debug_handler,
      /* user_data */ nullptr);
    /// ToDo: Create _debug_handle.
    create_debug_report_callback_ext(
      _instance,
      &static_cast<const VkDebugReportCallbackCreateInfoEXT&>(
        debug_create_info),
      nullptr, &_debug_handle);
  }

  _get_physical_device_surface_support_khr =
    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(
      proc_address("vkGetPhysicalDeviceSurfaceSupportKHR"));
  _get_physical_device_surface_capabilities_khr =
    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(
      proc_address("vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
  _get_physical_device_surface_formats_khr =
    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(
      proc_address("vkGetPhysicalDeviceSurfaceFormatsKHR"));
  _get_physical_device_surface_present_modes_khr =
    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(
      proc_address("vkGetPhysicalDeviceSurfacePresentModesKHR"));

  std::uint32_t physical_device_count = 0;
  std::vector<VkPhysicalDevice> physical_device_handles;
  vk_check(
    vk::enumerate_physical_devices(_instance, &physical_device_count, nullptr));
  if (physical_device_count == 0)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info(
           "vkEnumeratePhysicalDevices reported zero accessible "
           "devices.\n\nDo you have a compatible Vulkan installable client "
           "driver (ICD) installed?\nPlease look at the Getting Started "
           "guide for additional information.")
      << shift::core::context_info("vkEnumeratePhysicalDevices"));
  }
  physical_device_handles.resize(physical_device_count);
  vk_check(vk::enumerate_physical_devices(_instance, &physical_device_count,
                                          physical_device_handles.data()));
  _enabled_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  for (auto physical_device_handle : physical_device_handles)
  {
    // Look for device extensions.
    std::vector<vk::extension_properties> device_extensions;
    uint32_t device_extension_count = 0;
    vk_check(vk::enumerate_device_extension_properties(
      physical_device_handle, nullptr, &device_extension_count, nullptr));
    if (device_extension_count > 0)
    {
      device_extensions.resize(device_extension_count);
      vk_check(vk::enumerate_device_extension_properties(
        physical_device_handle, nullptr, &device_extension_count,
        device_extensions.data()));
    }
    bool found_all_enabled_extensions = true;
    for (const auto enabled_extension : _enabled_extensions)
    {
      bool found_extension = false;
      for (const auto& device_extension : device_extensions)
      {
        if (device_extension.extension_name().data() ==
            std::string{enabled_extension})
        {
          found_extension = true;
          break;
        }
      }
      if (!found_extension)
      {
        found_all_enabled_extensions = false;
        break;
      }
    }
    if (found_all_enabled_extensions)
    {
      _available_physical_devices.push_back(
        _physical_device_pool.construct(physical_device_handle));
    }
  }
  if (_available_physical_devices.empty())
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info(
           "vkEnumeratePhysicalDevices reported no devices which are "
           "suitable in the context of this application.\n\nDo you have a "
           "compatible Vulkan installable client driver (ICD) "
           "installed?\nPlease look at the Getting Started guide for "
           "additional information.")
      << shift::core::context_info("vkEnumeratePhysicalDevices"));
  }
}

instance::~instance()
{
  if (_debug_handle)
  {
    destroy_debug_report_callback_ext(_instance, _debug_handle, nullptr);
    _debug_handle = nullptr;
  }
  if (_instance)
  {
    vk::destroy_instance(_instance, _allocator);
    _instance = nullptr;
  }
}

vk::result instance::create_debug_report_callback_ext(
  VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* create_info,
  const VkAllocationCallbacks* allocator, VkDebugReportCallbackEXT* callback)
{
  return static_cast<vk::result>(_create_debug_report_callback_ext(
    instance, create_info, allocator, callback));
}

void instance::destroy_debug_report_callback_ext(
  VkInstance instance, VkDebugReportCallbackEXT callback,
  const VkAllocationCallbacks* allocator)
{
  _destroy_debug_report_callback_ext(instance, callback, allocator);
}

void instance::debug_report_message_ext(
  VkInstance instance, VkDebugReportFlagsEXT flags,
  VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location,
  int32_t message_code, const char* layer_prefix, const char* message)
{
  _debug_report_message_ext(instance, flags, object_type, object, location,
                            message_code, layer_prefix, message);
}

vk::result instance::get_physical_device_surface_support_khr(
  VkPhysicalDevice physical_device, uint32_t queue_family_index,
  VkSurfaceKHR surface, VkBool32* supported)
{
  return static_cast<vk::result>(_get_physical_device_surface_support_khr(
    physical_device, queue_family_index, surface, supported));
}

vk::result instance::get_physical_device_surface_capabilities_khr(
  VkPhysicalDevice physical_device, VkSurfaceKHR surface,
  VkSurfaceCapabilitiesKHR* surface_capabilities)
{
  return static_cast<vk::result>(_get_physical_device_surface_capabilities_khr(
    physical_device, surface, surface_capabilities));
}

vk::result instance::get_physical_device_surface_formats_khr(
  VkPhysicalDevice physical_device, VkSurfaceKHR surface,
  uint32_t* surface_format_count, VkSurfaceFormatKHR* surface_formats)
{
  return static_cast<vk::result>(_get_physical_device_surface_formats_khr(
    physical_device, surface, surface_format_count, surface_formats));
}

vk::result instance::get_physical_device_surface_present_modes_khr(
  VkPhysicalDevice physical_device, VkSurfaceKHR surface,
  uint32_t* present_mode_count, VkPresentModeKHR* present_modes)
{
  return static_cast<vk::result>(_get_physical_device_surface_present_modes_khr(
    physical_device, surface, present_mode_count, present_modes));
}

PFN_vkVoidFunction instance::proc_address(const char* name)
{
  if (auto result = vk::get_instance_proc_addr(_instance, name))
    return result;
  else
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Unable to find Vulkan function address.")
      << shift::core::context_info(name));
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL instance::debug_handler(
  VkFlags message_flags, VkDebugReportObjectTypeEXT /*object_type*/,
  uint64_t /*source_object*/, size_t /*location*/, int32_t code,
  const char* layer_prefix, const char* message, void* /*user_data*/)
{
  std::stringstream message_stream;
  message_stream << "Vulkan " << layer_prefix << " message: " << message
                 << " (code: " << code << ")";
  if ((message_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0)
  {
    log::error() << message_stream.str();
    if (platform::environment::is_debugger_present())
      platform::environment::debug_break();
  }
  else if ((message_flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0)
    log::warning() << message_stream.str();
  else
    log::debug() << message_stream.str();

  // VK_FALSE indicates that layer should not bail-out of an API call that had
  // validation failures. This may mean that the app dies inside the driver due
  // to invalid parameter(s). That's what would happen without validation
  // layers, so we'll keep that behavior here.
  return VK_FALSE;
}
}
