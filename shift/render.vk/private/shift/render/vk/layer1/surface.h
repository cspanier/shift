#ifndef SHIFT_RENDER_VK_LAYER1_SURFACE_H
#define SHIFT_RENDER_VK_LAYER1_SURFACE_H

#include <vector>
#include "shift/render/vk/shared.h"

namespace shift::render::vk::layer1
{
class instance;
class physical_device;

///
class surface
{
public:
  /// Constructor.
  /// @param surface_handle
  ///   An externally created surface handle.
  /// @remarks
  ///   The class takes ownership of the passed handle if construction succeeds.
  surface(vk::layer1::instance& instance,
          const vk::layer1::physical_device& physical_device,
          VkSurfaceKHR surface_handle);

  surface(const surface&) = delete;

  /// Move constructor.
  surface(surface&& other) noexcept;

  /// Destructor.
  ~surface();

  surface& operator=(const surface&) = delete;

  /// Move assignment operator.
  surface& operator=(surface&& other) noexcept;

  ///
  const vk::layer1::physical_device& physical_device() const
  {
    return *_physical_device;
  }

  ///
  VkSurfaceKHR& handle() noexcept
  {
    return _surface;
  }

  ///
  const VkSurfaceKHR& handle() const noexcept
  {
    return _surface;
  }

  ///
  vk::format format() const
  {
    return _format;
  }

  ///
  vk::color_space_khr color_space() const
  {
    return _color_space;
  }

  ///
  void capabilities(vk::surface_capabilities_khr& result);

  ///
  void available_present_modes(std::vector<vk::present_mode_khr>& result);

private:
  const vk::layer1::physical_device* _physical_device = nullptr;
  VkSurfaceKHR _surface = nullptr;
  vk::format _format = vk::format::undefined;
  vk::color_space_khr _color_space =
    vk::color_space_khr::color_space_srgb_nonlinear_khr;
};
}

#endif
