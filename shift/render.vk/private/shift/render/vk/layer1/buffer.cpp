#include "shift/render/vk/layer1/buffer.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/renderer_impl.h"

namespace shift::render::vk::layer1
{
buffer::buffer(vk::layer1::device& device,
               const vk::buffer_create_info& create_info)
: _device(&device), _size(create_info.size()), _usage(create_info.usage())
{
  vk_check(vk::create_buffer(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_buffer));
}

buffer::~buffer()
{
  if (_buffer != nullptr)
  {
    free_storage();
    vk::destroy_buffer(_device->handle(), _buffer,
                       renderer_impl::singleton_instance().default_allocator());
    _buffer = nullptr;
  }
}

void buffer::allocate_storage(memory_pool pool)
{
  vk::memory_requirements memory_requirements;
  vk::get_buffer_memory_requirements(_device->handle(), _buffer,
                                     &memory_requirements);

  if (_usage & vk::buffer_usage_flag::uniform_buffer_bit)
  {
    memory_requirements.alignment(
      std::max(memory_requirements.alignment(),
               _device->physical_device()
                 .properties()
                 .limits()
                 .min_uniform_buffer_offset_alignment()));
  }
  if (_usage & vk::buffer_usage_flag::storage_buffer_bit)
  {
    memory_requirements.alignment(
      std::max(memory_requirements.alignment(),
               _device->physical_device()
                 .properties()
                 .limits()
                 .min_storage_buffer_offset_alignment()));
  }

  _memory = _device->memory_manager().allocate(pool, memory_requirements);
}

void buffer::bind_storage()
{
  BOOST_ASSERT(_memory);
  if (_memory)
  {
    vk_check(vk::bind_buffer_memory(_device->handle(), _buffer,
                                    _memory.handle(), _memory.offset()));
  }
}

void buffer::free_storage()
{
  if (_memory)
    _device->memory_manager().free(_memory);
}

void* buffer::address(std::uint64_t offset, std::uint64_t size)
{
  if (size == 0)
    size = _memory.size() - offset;
  return _memory.address(offset, size);
}

void buffer::flush_memory(std::uint64_t offset, std::uint64_t size)
{
  if (size == 0)
    size = _memory.size() - offset;
  _memory.flush(static_cast<std::uint32_t>(offset),
                static_cast<std::uint32_t>(size));
}
}
