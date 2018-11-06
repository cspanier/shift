#include "shift/render/vk/memory_chunk.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"
#include <algorithm>

namespace shift::render::vk
{
memory_chunk::memory_chunk(vk::layer1::device& device,
                           const vk::memory_allocate_info& allocation_info,
                           vk::memory_property_flags property_flags)
: device(&device),
  property_flags(property_flags),
  total_size(static_cast<std::uint32_t>(allocation_info.allocation_size()))
{
  vk_check(vk::allocate_memory(
    device.handle(), &allocation_info,
    renderer_impl::singleton_instance().default_allocator(), &memory));

  if (!memory)
  {
    SHIFT_THROW_EXCEPTION(core::runtime_error() << core::context_info(
                            "Error allocating device memory."));
  }
  _free_list.push_back({0, total_size});
  if (property_flags & vk::memory_property_flag::host_visible_bit)
  {
    vk_check(
      vk::map_memory(device.handle(), memory, 0, VK_WHOLE_SIZE, 0, &_address));
  }
}

memory_chunk::memory_chunk(memory_chunk&& other) noexcept
: device(std::move(other.device)),
  memory(std::move(other.memory)),
  property_flags(std::move(other.property_flags)),
  total_size(std::move(other.total_size)),
  used_bytes(std::move(other.used_bytes)),
  _address(std::move(other._address)),
  _free_list(std::move(other._free_list))
{
  other.memory = nullptr;
  other._address = nullptr;
}

memory_chunk::~memory_chunk()
{
  if (memory)
  {
    BOOST_ASSERT(used_bytes == 0);
    if (_address)
    {
      vk::unmap_memory(device->handle(), memory);
      _address = nullptr;
    }
    vk::free_memory(device->handle(), memory,
                    renderer_impl::singleton_instance().default_allocator());
    memory = nullptr;
  }
}

memory_chunk& memory_chunk::operator=(memory_chunk&& other) noexcept
{
  device = std::move(other.device);
  memory = std::move(other.memory);
  property_flags = std::move(other.property_flags);
  total_size = std::move(other.total_size);
  used_bytes = std::move(other.used_bytes);
  _address = std::move(other._address);
  _free_list = std::move(other._free_list);
  other.memory = nullptr;
  other._address = nullptr;
  return *this;
}

bool memory_chunk::allocate(std::uint32_t size, std::uint32_t alignment,
                            memory_block& result)
{
  if (total_size - used_bytes < size)
    return false;

  for (auto free_block_iter = _free_list.begin();
       free_block_iter != _free_list.end(); ++free_block_iter)
  {
    auto begin_offset =
      (free_block_iter->offset + alignment - 1) / alignment * alignment;
    auto end_offset = begin_offset + size;
    auto size_lhs = begin_offset - free_block_iter->offset;
    auto size_rhs = free_block_iter->size - (size_lhs + size);
    if (free_block_iter->size >= size_lhs + size)
    {
      // Eventually insert new free blocks of size_lhs and size_rhs bytes.
      // Note that the order of blocks is maintained by later removing
      // free_block_iter.
      if (size_lhs > 0)
      {
        _free_list.insert(free_block_iter,
                          memory_range{free_block_iter->offset, size_lhs});
      }
      if (size_rhs > 0)
      {
        _free_list.insert(free_block_iter, memory_range{end_offset, size_rhs});
      }
      used_bytes += size;
      // Erase this free block.
      _free_list.erase(free_block_iter);

      result = memory_block(*this, begin_offset, size);
      return true;
    }
  }
  return false;
}

void memory_chunk::free(memory_block& block)
{
  // Look for another free block to merge with.
  bool merged = false;
  auto free_block_iter = _free_list.begin();
  for (; free_block_iter != _free_list.end(); ++free_block_iter)
  {
    if (free_block_iter->offset + free_block_iter->size == block.offset())
    {
      auto next_free_block_iter = free_block_iter;
      ++next_free_block_iter;
      // Check whether we have to merge with left and right block, or only
      // with left one.
      if (next_free_block_iter != _free_list.end() &&
          block.offset() + block.size() == next_free_block_iter->offset)
      {
        free_block_iter->size += block.size() + next_free_block_iter->size;
        _free_list.erase(next_free_block_iter);
      }
      else
        free_block_iter->size += block.size();
      merged = true;
      break;
    }
    else if (block.offset() + block.size() == free_block_iter->offset)
    {
      // Merge chunk with the right free block.
      free_block_iter->offset = block.offset();
      free_block_iter->size += block.size();
      merged = true;
      break;
    }
    else if (free_block_iter->offset > block.offset())
      break;
  }
  // If not merged, insert into free list.
  if (!merged)
  {
    _free_list.insert(free_block_iter,
                      memory_range{block.offset(), block.size()});
  }
  used_bytes -= block.size();

  // Reset block.
  block = memory_block{};
}

void* memory_chunk::address(VkDeviceSize offset,
                            [[maybe_unused]] VkDeviceSize size)
{
  if (_address)
    return static_cast<std::byte*>(_address) + offset;
  else
    return nullptr;
}

void memory_chunk::flush(std::uint32_t offset, std::uint32_t size)
{
  BOOST_ASSERT(_address);
  vk::flush_mapped_memory_ranges(device->handle(), 1,
                                 core::rvalue_address(vk::mapped_memory_range(
                                   /* next */ nullptr,
                                   /* memory */ memory,
                                   /* offset */ offset,
                                   /* size */ size)));
}
}
