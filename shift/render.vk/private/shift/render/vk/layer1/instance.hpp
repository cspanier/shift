#ifndef SHIFT_RENDER_VK_LAYER1_INSTANCE_HPP
#define SHIFT_RENDER_VK_LAYER1_INSTANCE_HPP

#include <vector>
#include <boost/pool/object_pool.hpp>
#include <shift/core/bit_field.hpp>
#include <shift/core/singleton.hpp>
#include <shift/render/vk/renderer.hpp>
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/layer1/physical_device.hpp"

namespace shift::render::vk::layer1
{
///
/// @remarks
///   The type is not moveable because several other types hold a pointer to
///   their associated instance instance.
class instance
{
public:
  /// Constructor.
  instance(const vk::application_info& application_info,
           vk::debug_layers debug_layers, vk::debug_logs debug_logs,
           const vk::allocation_callbacks* allocator = nullptr);

  instance(const instance&) = delete;
  instance(instance&&) = delete;

  /// Destructor.
  ~instance();

  instance& operator=(const instance&) = delete;
  instance& operator=(instance&&) = delete;

  ///
  VkInstance& handle() noexcept
  {
    return _instance;
  }

  ///
  const VkInstance& handle() const noexcept
  {
    return _instance;
  }

  ///
  const std::vector<physical_device*>& available_physical_devices() const
  {
    return _available_physical_devices;
  }

  ///
  const std::vector<const char*>& enabled_layers() const
  {
    return _enabled_layers;
  }

  ///
  const std::vector<const char*>& enabled_extensions() const
  {
    return _enabled_extensions;
  }

  ///
  vk::result create_debug_report_callback_ext(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator, VkDebugReportCallbackEXT* callback);

  ///
  void destroy_debug_report_callback_ext(
    VkInstance instance, VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks* allocator);

  ///
  void debug_report_message_ext(VkInstance instance,
                                VkDebugReportFlagsEXT flags,
                                VkDebugReportObjectTypeEXT object_type,
                                uint64_t object, size_t location,
                                int32_t message_code, const char* layer_prefix,
                                const char* message);

  ///
  vk::result get_physical_device_surface_support_khr(
    VkPhysicalDevice physical_device, uint32_t queue_family_index,
    VkSurfaceKHR surface, VkBool32* supported);

  ///
  vk::result get_physical_device_surface_capabilities_khr(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR* surface_capabilities);

  ///
  vk::result get_physical_device_surface_formats_khr(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface,
    uint32_t* surface_format_count, VkSurfaceFormatKHR* surface_formats);

  ///
  vk::result get_physical_device_surface_present_modes_khr(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface,
    uint32_t* present_mode_count, VkPresentModeKHR* present_modes);

private:
  struct per_thread_t
  {
  };

  ///
  PFN_vkVoidFunction proc_address(const char* name);

  ///
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debug_handler(VkFlags message_flags, VkDebugReportObjectTypeEXT object_type,
                uint64_t source_object, size_t location, int32_t code,
                const char* layer_prefix, const char* message, void* user_data);

  VkInstance _instance = nullptr;
  const vk::allocation_callbacks* _allocator = nullptr;
  VkDebugReportCallbackEXT _debug_handle = nullptr;
  boost::object_pool<physical_device> _physical_device_pool;
  std::vector<physical_device*> _available_physical_devices;
  std::vector<const char*> _enabled_layers;
  std::vector<const char*> _enabled_extensions;

  PFN_vkCreateDebugReportCallbackEXT _create_debug_report_callback_ext =
    nullptr;
  PFN_vkDestroyDebugReportCallbackEXT _destroy_debug_report_callback_ext =
    nullptr;
  PFN_vkDebugReportMessageEXT _debug_report_message_ext = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR
    _get_physical_device_surface_support_khr = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
    _get_physical_device_surface_capabilities_khr = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR
    _get_physical_device_surface_formats_khr = nullptr;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR
    _get_physical_device_surface_present_modes_khr = nullptr;
};
}

#endif
