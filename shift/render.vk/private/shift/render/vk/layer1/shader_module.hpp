#ifndef SHIFT_RENDER_VK_LAYER1_SHADERMODULE_HPP
#define SHIFT_RENDER_VK_LAYER1_SHADERMODULE_HPP

#include <vector>
#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;

/// @remarks
///   A shader module object is only used during pipeline creation any may
///   be deleted immediately afterwards.
class shader_module
{
public:
  /// Default constructor.
  shader_module() = default;

  /// Constructor.
  shader_module(vk::layer1::device& device,
                const shader_module_create_info& create_info);

  shader_module(const shader_module&) = delete;
  shader_module(shader_module&&) = delete;

  /// Destructor.
  ~shader_module();

  shader_module& operator=(const shader_module&) = delete;
  shader_module& operator=(shader_module&&) = delete;

  ///
  VkShaderModule& handle() noexcept
  {
    return _shader_module;
  }

  ///
  const VkShaderModule& handle() const noexcept
  {
    return _shader_module;
  }

private:
  device* _device = nullptr;
  VkShaderModule _shader_module = nullptr;
};
}

#endif
