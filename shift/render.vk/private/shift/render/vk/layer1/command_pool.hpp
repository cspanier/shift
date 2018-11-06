#ifndef SHIFT_RENDER_VK_LAYER1_COMMANDPOOL_HPP
#define SHIFT_RENDER_VK_LAYER1_COMMANDPOOL_HPP

#include <vector>
#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;

///
/// @remarks
///   The type is not moveable because command_buffer holds a pointer to its
///   associated command_pool instance.
class command_pool
{
public:
  /// Default constructor.
  command_pool() noexcept;

  /// Constructor.
  command_pool(vk::layer1::device& device, std::uint32_t queue_family_index,
               command_pool_create_flags create_flags) noexcept;

  command_pool(const command_pool&) = delete;
  command_pool(command_pool&&) = delete;

  /// Destructor.
  ~command_pool();

  command_pool& operator=(const command_pool&) = delete;
  command_pool& operator=(command_pool&&) = delete;

  ///
  vk::layer1::device& device() const noexcept
  {
    return *_device;
  }

  ///
  VkCommandPool& handle() noexcept
  {
    return _command_pool;
  }

  ///
  const VkCommandPool& handle() const noexcept
  {
    return _command_pool;
  }

private:
  vk::layer1::device* _device = nullptr;
  VkCommandPool _command_pool = nullptr;
};
}

#endif
