#ifndef SHIFT_RENDER_VK_MEMORY_BLOCK_H
#define SHIFT_RENDER_VK_MEMORY_BLOCK_H

#include <array>
#include <vector>
#include <list>
#include <shift/core/boost_disable_warnings.h>
#include <boost/container/slist.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/types.h>
#include "shift/render/vk/shared.h"

namespace shift::render::vk
{
// Several memory types each referencing one of several memory heaps.
// There are typically only one or two heaps, but several types.
// Each allocation of some type reserves memory from one of the heaps.

class memory_chunk;

///
struct memory_block
{
  ///
  memory_block() = default;

  ///
  memory_block(memory_chunk& chunk, std::uint32_t offset, std::uint32_t size);

  memory_block(const memory_block&) = delete;

  ///
  memory_block(memory_block&& other) noexcept;

  memory_block& operator=(const memory_block&) = delete;

  ///
  memory_block& operator=(memory_block&& other) noexcept;

  ///
  operator bool() const noexcept;

  ///
  VkDeviceMemory handle() const noexcept;

  ///
  memory_chunk* chunk() const noexcept;

  ///
  void* address(VkDeviceSize offset, VkDeviceSize size);

  ///
  void flush(std::uint32_t offset, std::uint32_t size);

  ///
  std::uint32_t offset() const
  {
    return _offset;
  }

  ///
  std::uint32_t size() const
  {
    return _size;
  }

private:
  memory_chunk* _chunk = nullptr;
  std::uint32_t _offset = 0;
  std::uint32_t _size = 0;
};
}

#endif
