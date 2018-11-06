#ifndef SHIFT_RENDER_VK_LAYER1_BUFFER_HPP
#define SHIFT_RENDER_VK_LAYER1_BUFFER_HPP

#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/memory_manager.hpp"

namespace shift::render::vk::layer1
{
class device;

///
/// @remarks
///   The type is not moveable because buffer_view holds a pointer to its
///   associated buffer instance.
class buffer
{
public:
  /// Constructor.
  buffer(vk::layer1::device& device, const vk::buffer_create_info& create_info);

  buffer(const buffer&) = delete;
  buffer(buffer&&) = delete;

  /// Destructor.
  ~buffer();

  buffer& operator=(const buffer&) = delete;
  buffer& operator=(buffer&&) = delete;

  ///
  vk::layer1::device& device()
  {
    return *_device;
  }

  ///
  VkBuffer& handle()
  {
    return _buffer;
  }

  ///
  const VkBuffer& handle() const
  {
    return _buffer;
  }

  ///
  std::uint64_t size() const
  {
    return _size;
  }

  ///
  vk::buffer_usage_flags usage()
  {
    return _usage;
  }

  ///
  void allocate_storage(memory_pool pool);

  ///
  void bind_storage();

  ///
  void free_storage();

  ///
  void* address(std::uint64_t offset = 0, std::uint64_t size = 0);

  /// Flush the specified memory region.
  void flush_memory(std::uint64_t offset = 0, std::uint64_t size = 0);

private:
  vk::layer1::device* _device = nullptr;
  VkBuffer _buffer = nullptr;
  std::uint64_t _size = 0;
  vk::buffer_usage_flags _usage;
  memory_block _memory;
};
}

#endif
