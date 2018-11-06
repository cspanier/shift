#ifndef SHIFT_RENDER_VK_MEMORY_CHUNK_HPP
#define SHIFT_RENDER_VK_MEMORY_CHUNK_HPP

#include <array>
#include <vector>
#include <list>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/container/slist.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/core/types.hpp>
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/memory_block.hpp"

namespace shift::render::vk::layer1
{
class device;
class physical_device;
}

namespace shift::render::vk
{
struct memory_range
{
  std::uint32_t offset;
  std::uint32_t size;
};

///
class memory_chunk
{
public:
  static constexpr std::size_t max_size()
  {
    using namespace shift::core::literals;

    return 64_MiB;
  }

  /// Constructor.
  memory_chunk(vk::layer1::device& device,
               const vk::memory_allocate_info& allocation_info,
               vk::memory_property_flags property_flags);

  memory_chunk(const memory_chunk&) = delete;

  /// Move constructor.
  memory_chunk(memory_chunk&& other) noexcept;

  /// Destructor.
  ~memory_chunk();

  memory_chunk& operator=(const memory_chunk&) = delete;

  /// Move assignment operator.
  memory_chunk& operator=(memory_chunk&& other) noexcept;

  ///
  bool allocate(std::uint32_t size, std::uint32_t alignment,
                memory_block& result);

  ///
  void free(memory_block& block);

  ///
  void* address(VkDeviceSize offset, VkDeviceSize size);

  ///
  void flush(std::uint32_t offset, std::uint32_t size);

  vk::layer1::device* device = nullptr;
  VkDeviceMemory memory = nullptr;
  vk::memory_property_flags property_flags;
  std::uint32_t total_size;
  std::uint32_t used_bytes = 0;

private:
  void* _address = nullptr;

  std::list<memory_range> _free_list;
};
}

#endif
