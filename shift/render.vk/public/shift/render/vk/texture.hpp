#ifndef SHIFT_RENDER_VK_TEXTURE_HPP
#define SHIFT_RENDER_VK_TEXTURE_HPP

#include "shift/render/vk/types.hpp"
#include "shift/render/vk/shared_object.hpp"

namespace shift::render::vk
{
///
class texture : public shared_object
{
public:
  texture() = default;
  texture(const texture&) = delete;
  texture(texture&&) = delete;
  ~texture() noexcept override = 0;
  texture& operator=(const texture&) = delete;
  texture& operator=(texture&&) = delete;

  /// Returns the number of mip levels that the texture has been created with.
  virtual std::uint32_t mip_levels() noexcept = 0;

  /// Returns the number of array layers that the texture has been created with.
  virtual std::uint32_t array_layers() noexcept = 0;
};
}

#endif
