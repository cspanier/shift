#include "shift/render/vk/layer2/buffer.h"
#include "shift/render/vk/layer2/mesh.h"
#include "shift/render/vk/layer1/command_buffer.h"
#include "shift/render/vk/shared.h"
#include <shift/resource/buffer.h>

namespace shift::render::vk
{
buffer::~buffer() noexcept
{
}
}

namespace shift::render::vk::layer2
{
buffer::buffer(vk::layer1::device& device, std::uint64_t size,
               vk::buffer_usage_flags usage,
               std::shared_ptr<resource::buffer> source)
: _device(&device), _size(size), _usage(usage), _source(std::move(source))
{
  _buffer = vk::make_framed_shared<vk::layer1::buffer>(
    *_device, vk::buffer_create_info(
                /* next */ nullptr,
                /* flags */ vk::buffer_create_flag::none,
                /* size */ _size,
                /* usage */ _usage | vk::buffer_usage_flag::transfer_dst_bit,
                /* sharing_mode */ vk::sharing_mode::exclusive,
                /* queue_family_index_count */ 0,
                /* queue_family_indices */ nullptr));
  _buffer->allocate_storage(memory_pool::resource_buffers);
  _buffer->bind_storage();

  _staging_buffer = vk::make_framed_shared<vk::layer1::buffer>(
    *_device, vk::buffer_create_info(
                /* next */ nullptr,
                /* flags */ vk::buffer_create_flag::none,
                /* size */ _size,
                /* usage */ vk::buffer_usage_flag::transfer_src_bit,
                /* sharing_mode */ vk::sharing_mode::exclusive,
                /* queue_family_index_count */ 0,
                /* queue_family_indices */ nullptr));
  _staging_buffer->allocate_storage(memory_pool::staging_buffers);
  _staging_buffer->bind_storage();
}

buffer::~buffer() noexcept
{
}

const std::shared_ptr<resource::buffer>& buffer::source_buffer() const
{
  return _source;
}

std::uint64_t buffer::size() noexcept
{
  return _size;
}

void buffer::copy_to_staging_buffer()
{
  BOOST_ASSERT(_source != nullptr);

  std::memcpy(_staging_buffer->address(0, 0), _source->storage.data(),
              _source->storage.size());
}

void buffer::copy_to_device_memory(vk::layer1::command_buffer& command_buffer)
{
  command_buffer.copy_buffer(*_staging_buffer, *_buffer, 1,
                             core::rvalue_address(vk::buffer_copy(
                               /* src_offset */ 0,
                               /* dst_offset */ 0,
                               /* size */ _size)));

  /// ToDo: Do we need this? We synchronize with the host anyway before using
  /// the buffer and as such the buffer _should_ be synchronized between
  /// transfer and graphics queues as well.
  //  command_buffer.pipeline_barrier(
  //    vk::pipeline_stage_flag::host_bit,
  //    vk::pipeline_stage_flag::vertex_input_bit, vk::dependency_flag::none,
  //    vk::buffer_memory_barrier(
  //      /* next */ nullptr,
  //      /* src_access_mask */ vk::access_flag::host_write_bit,
  //      /* dst_access_mask */ vk::access_flag::vertex_attribute_read_bit,
  //      /* src_queue_family_index */ _device->transfer_queue_family_index(),
  //      /* dst_queue_family_index */ _device->graphics_queue_family_index(),
  //      /* buffer */ _buffer->handle(),
  //      /* offset */ 0,
  //      /* size */ VK_WHOLE_SIZE));
}
}
