#ifndef SHIFT_RENDER_VK_MEMORY_MANAGER_HPP
#define SHIFT_RENDER_VK_MEMORY_MANAGER_HPP

#include <array>
#include <vector>
#include <list>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/container/slist.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/core/types.hpp>
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/memory_chunk.hpp"

namespace shift::render::vk
{
///
struct memory_allocator
{
  ///
  memory_allocator(vk::memory_property_flags flags);

  memory_allocator(const memory_allocator&) = delete;

  ///
  memory_allocator(memory_allocator&& other) noexcept;

  memory_allocator& operator=(const memory_allocator&) = delete;

  ///
  memory_allocator& operator=(memory_allocator&& other) noexcept;

  vk::memory_property_flags property_flags;
  std::vector<std::unique_ptr<memory_chunk>> memory_chunks;
};

enum memory_pool : std::uint32_t
{
  staging_buffers,
  staging_images,
  resource_buffers,
  resource_images,
  pipeline_buffers,
  pipeline_images,

  memory_pool_count
};

/// The memory manager holds one memory allocator per supported memory type.
class memory_manager
{
public:
  /// Constructor.
  memory_manager(vk::layer1::device& device,
                 const vk::layer1::physical_device& physical_device);

  memory_manager(const memory_manager&) = delete;
  memory_manager(memory_manager&&) = delete;

  /// Destructor.
  ~memory_manager();

  memory_manager& operator=(const memory_manager&) = delete;
  memory_manager& operator=(memory_manager&&) = delete;

  /// Releases all memory.
  /// @pre
  ///   All memory blocks returned by alloc are deallocated using free.
  void clear();

  ///
  memory_block allocate(memory_pool pool,
                        vk::memory_requirements memory_requirements);

  ///
  void free(memory_block& block);

private:
  std::uint32_t memory_type_from_properties(
    std::uint32_t type_bits, vk::memory_property_flags requirements_mask);

  vk::layer1::device* _device = nullptr;
  const vk::layer1::physical_device* _physical_device = nullptr;
  vk::physical_device_memory_properties _memory_properties;

  std::array<std::vector<memory_allocator>, memory_pool_count>
    _memory_allocator_pools;
};
}

#endif
