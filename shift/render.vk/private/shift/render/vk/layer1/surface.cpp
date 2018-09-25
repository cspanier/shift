#include "shift/render/vk/layer1/surface.h"
#include "shift/render/vk/layer1/physical_device.h"
#include "shift/render/vk/layer1/instance.h"
#include "shift/render/vk/renderer_impl.h"

namespace shift::render::vk::layer1
{
surface::surface(vk::layer1::instance& instance,
                 const vk::layer1::physical_device& physical_device,
                 VkSurfaceKHR surface_handle)
: _physical_device(&physical_device), _surface(surface_handle)
{
  // Get the list of vk::format's that are supported:
  uint32_t surface_format_count;
  vk_check(instance.get_physical_device_surface_formats_khr(
    _physical_device->handle(), _surface, &surface_format_count, nullptr));
  std::vector<vk::surface_format_khr> surface_formats;
  surface_formats.resize(surface_format_count);
  vk_check(instance.get_physical_device_surface_formats_khr(
    _physical_device->handle(), _surface, &surface_format_count,
    reinterpret_cast<VkSurfaceFormatKHR*>(surface_formats.data())));
  if (surface_formats.empty())
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info(
           "getPhysicalDeviceSurfaceFormatsKHR didn't return any formats.")
      << shift::core::context_info(
           _physical_device->properties().device_name().data()));
  }
  else if (surface_formats.size() == 1 &&
           surface_formats.front().format() == vk::format::undefined)
  {
    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned.
    _format = vk::format::b8_g8_r8_a8_unorm;
    _color_space = surface_formats.front().color_space();
  }
  else
  {
    /// ToDo: Chose a format.
    _format = surface_formats.front().format();
    _color_space = surface_formats.front().color_space();
  }
}

surface::surface(surface&& other) noexcept
: _physical_device(other._physical_device),
  _surface(other._surface),
  _format(other._format),
  _color_space(other._color_space)
{
  other._surface = nullptr;
}

surface::~surface()
{
  if (_surface != nullptr)
  {
    auto& renderer = renderer_impl::singleton_instance();
    vk::destroy_surface_khr(renderer.instance().handle(), _surface,
                            renderer.default_allocator());
    _surface = nullptr;
  }
}

surface& surface::operator=(surface&& other) noexcept
{
  _physical_device = other._physical_device;
  _surface = other._surface;
  _format = other._format;
  _color_space = other._color_space;

  other._surface = nullptr;
  return *this;
}

void surface::capabilities(vk::surface_capabilities_khr& result)
{
  auto& renderer = renderer_impl::singleton_instance();
  vk_check(renderer.instance().get_physical_device_surface_capabilities_khr(
    _physical_device->handle(), _surface,
    reinterpret_cast<VkSurfaceCapabilitiesKHR*>(&result)));
}

void surface::available_present_modes(std::vector<vk::present_mode_khr>& result)
{
  auto& instance = renderer_impl::singleton_instance().instance();
  std::uint32_t present_mode_count = 0;
  vk_check(instance.get_physical_device_surface_present_modes_khr(
    _physical_device->handle(), _surface, &present_mode_count, nullptr));
  result.resize(present_mode_count);
  vk_check(instance.get_physical_device_surface_present_modes_khr(
    _physical_device->handle(), _surface, &present_mode_count,
    reinterpret_cast<VkPresentModeKHR*>(result.data())));
}
}
