#include "shift/render/vk/memory_manager.h"
#include "shift/render/vk/layer1/physical_device.h"
#include "shift/render/vk/layer1/device.h"
#include <algorithm>

namespace shift::render::vk
{
inline constexpr vk::memory_property_flags memory_pool_flags(memory_pool pool)
{
  BOOST_ASSERT(pool < memory_pool::memory_pool_count);
  constexpr vk::memory_property_flags flags[memory_pool::memory_pool_count] = {
    // staging_buffers:
    vk::memory_property_flag::host_visible_bit,
    // staging_images:
    vk::memory_property_flag::host_visible_bit,

    // resource_buffers:
    vk::memory_property_flag::device_local_bit,
    // vk::memory_property_flag::host_visible_bit |
    //  vk::memory_property_flag::host_coherent_bit,

    // resource_images:
    vk::memory_property_flag::device_local_bit,

    // pipeline_buffers:
    vk::memory_property_flag::host_visible_bit |
      vk::memory_property_flag::host_coherent_bit,
    // pipeline_images:
    vk::memory_property_flag::device_local_bit};
  return flags[pool];
}

memory_allocator::memory_allocator(vk::memory_property_flags flags)
: property_flags(std::move(flags))
{
}

memory_allocator::memory_allocator(memory_allocator&& other) noexcept
: property_flags(std::move(other.property_flags)),
  memory_chunks(std::move(other.memory_chunks))
{
}

memory_allocator& memory_allocator::operator=(memory_allocator&& other) noexcept
{
  property_flags = std::move(other.property_flags);
  memory_chunks = std::move(other.memory_chunks);
  return *this;
}

memory_manager::memory_manager(
  vk::layer1::device& device,
  const vk::layer1::physical_device& physical_device)
{
  _device = &device;
  _physical_device = &physical_device;
  vk::get_physical_device_memory_properties(_physical_device->handle(),
                                            &_memory_properties);

  for (auto& memory_allocator_pool : _memory_allocator_pools)
  {
    for (std::uint32_t memory_type = 0u;
         memory_type < _memory_properties.memory_type_count(); ++memory_type)
    {
      memory_allocator_pool.emplace_back(
        _memory_properties.memory_types()[memory_type].property_flags());
    }
  }
}

memory_manager::~memory_manager()
{
  clear();
}

void memory_manager::clear()
{
  for (auto& memory_allocator_pool : _memory_allocator_pools)
  {
    for (auto& memory_allocator : memory_allocator_pool)
      memory_allocator.memory_chunks.clear();
  }
}

memory_block memory_manager::allocate(
  memory_pool pool, vk::memory_requirements memory_requirements)
{
  using std::max;
  using std::min;

  memory_block result;
  auto memory_type = memory_type_from_properties(
    memory_requirements.memory_type_bits(), memory_pool_flags(pool));
  BOOST_ASSERT(memory_type < _memory_allocator_pools[pool].size());
  auto& memory_allocator = _memory_allocator_pools[pool][memory_type];
  // Try to find a free block in one of the existing memory chunks.
  for (auto& chunk : memory_allocator.memory_chunks)
  {
    if (chunk->allocate(
          static_cast<std::uint32_t>(memory_requirements.size()),
          static_cast<std::uint32_t>(memory_requirements.alignment()), result))
    {
      return result;
    }
  }
  const auto& memory_type_properties =
    _memory_properties.memory_types()[memory_type];
  const auto& memory_heap_properties =
    _memory_properties.memory_heaps()[memory_type_properties.heap_index()];
  // Allocate a new chunk of memory.
  auto new_chunk = memory_allocator.memory_chunks.insert(
    memory_allocator.memory_chunks.end(),
    std::make_unique<memory_chunk>(
      *_device,
      vk::memory_allocate_info(
        /* next */ nullptr,
        /* allocation_size */
        max(min(memory_heap_properties.size() / 16, memory_chunk::max_size()),
            memory_requirements.size()),
        /* memory_type_index */ memory_type),
      memory_allocator.property_flags));
  if (!(*new_chunk)
         ->allocate(static_cast<std::uint32_t>(memory_requirements.size()),
                    static_cast<std::uint32_t>(memory_requirements.alignment()),
                    result))
  {
    SHIFT_THROW_EXCEPTION(core::runtime_error()
                          << core::context_info("Memory allocation error"));
  }
  return result;
}

void memory_manager::free(memory_block& block)
{
  BOOST_ASSERT(block);
  if (!block)
    return;
  block.chunk()->free(block);
}

std::uint32_t memory_manager::memory_type_from_properties(
  std::uint32_t type_bits, vk::memory_property_flags requirements_mask)
{
  // Search memtypes to find first index with those properties
  for (std::uint32_t i = 0; i < _memory_properties.memory_type_count();
       ++i, type_bits >>= 1)
  {
    if ((type_bits & 1) == 1)
    {
      // Type is available, does it match user properties?
      if ((_memory_properties.memory_types()[i].property_flags() &
           requirements_mask) == requirements_mask)
      {
        return i;
      }
    }
  }
  // No memory type matched, return invalid type index.
  BOOST_ASSERT(false);
  return std::numeric_limits<std::uint32_t>::max();
}
}
