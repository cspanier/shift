#include "shift/render/vk/memory_block.h"
#include "shift/render/vk/memory_chunk.h"
#include <algorithm>

namespace shift::render::vk
{
memory_block::memory_block(memory_chunk& chunk, std::uint32_t offset,
                           std::uint32_t size)
: _chunk(&chunk), _offset(offset), _size(size)
{
}

memory_block::memory_block(memory_block&& other) noexcept
: _chunk(std::move(other._chunk)),
  _offset(std::move(other._offset)),
  _size(std::move(other._size))
{
  other._chunk = nullptr;
  other._offset = 0;
  other._size = 0;
}

memory_block& memory_block::operator=(memory_block&& other) noexcept
{
  _chunk = std::move(other._chunk);
  _offset = std::move(other._offset);
  _size = std::move(other._size);
  other._chunk = nullptr;
  other._offset = 0;
  other._size = 0;
  return *this;
}

memory_block::operator bool() const noexcept
{
  BOOST_ASSERT(!_chunk || _chunk->memory);
  return _chunk != nullptr;
}

VkDeviceMemory memory_block::handle() const noexcept
{
  if (_chunk)
    return _chunk->memory;
  else
    return nullptr;
}

memory_chunk* memory_block::chunk() const noexcept
{
  return _chunk;
}

void* memory_block::address(VkDeviceSize offset, VkDeviceSize size)
{
  BOOST_ASSERT(_chunk);
  return _chunk->address(_offset + offset, size);
}

void memory_block::flush(std::uint32_t offset, std::uint32_t size)
{
  BOOST_ASSERT(_chunk);
  _chunk->flush(_offset + offset, size);
}
}
