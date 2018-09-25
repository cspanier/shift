#include "shift/render/vk/layer1/command_buffer.h"
#include "shift/render/vk/layer1/command_pool.h"
#include "shift/render/vk/layer1/timestamp_query_pool.h"
#include "shift/render/vk/layer1/query_pool.h"
#include "shift/render/vk/layer1/buffer.h"
#include "shift/render/vk/layer1/framebuffer.h"
#include "shift/render/vk/layer1/render_pass.h"
#include "shift/render/vk/layer1/descriptor_set.h"
#include "shift/render/vk/layer1/pipeline_layout.h"
#include "shift/render/vk/layer1/pipeline.h"
#include "shift/render/vk/layer1/image.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/layer1/instance.h"
#include <shift/core/exception.h>
#include <gsl/gsl>
#include <cstdint>

namespace shift::render::vk::layer1
{
command_buffer::command_buffer(vk::layer1::command_pool& command_pool,
                               vk::command_buffer_level level)
: _command_pool(&command_pool)
{
  vk_check(vk::allocate_command_buffers(
    _command_pool->device().handle(),
    core::rvalue_address(vk::command_buffer_allocate_info(
      /* next */ nullptr,
      /* command_pool */ _command_pool->handle(),
      /* level */ level,
      /* command_buffer_count */ 1)),
    &_command_buffer));
}

command_buffer::~command_buffer()
{
  if (_command_buffer != nullptr)
  {
    vk::free_command_buffers(_command_pool->device().handle(),
                             _command_pool->handle(), 1, &_command_buffer);
    _command_buffer = nullptr;
  }
}

void command_buffer::begin(const command_buffer_begin_info& begin_info)
{
  vk_check(vk::begin_command_buffer(_command_buffer, &begin_info));
  _command_count = 0;
}

void command_buffer::end()
{
  vk_check(vk::end_command_buffer(_command_buffer));
}

void command_buffer::reset(vk::command_buffer_reset_flags reset_flags)
{
  vk_check(vk::reset_command_buffer(_command_buffer, reset_flags));
  _command_count = 0;
}

void command_buffer::clear_color_image(
  vk::layer1::image& image, vk::image_layout image_layout,
  const vk::clear_color_value& color, std::uint32_t range_count,
  const vk::image_subresource_range* ranges)
{
  vk::cmd_clear_color_image(_command_buffer, image.handle(), image_layout,
                            &color, range_count, ranges);
}

void command_buffer::clear_depth_stencil_image(
  vk::layer1::image& image, vk::image_layout image_layout,
  const vk::clear_depth_stencil_value& depth_stencil, std::uint32_t range_count,
  const vk::image_subresource_range* ranges)
{
  vk::cmd_clear_depth_stencil_image(_command_buffer, image.handle(),
                                    image_layout, &depth_stencil, range_count,
                                    ranges);
}

void command_buffer::copy_buffer(vk::layer1::buffer& source_buffer,
                                 vk::layer1::buffer& destination_buffer,
                                 std::uint32_t buffer_copy_region_count,
                                 const vk::buffer_copy* buffer_copy_regions)
{
  vk::cmd_copy_buffer(_command_buffer, source_buffer.handle(),
                      destination_buffer.handle(), buffer_copy_region_count,
                      buffer_copy_regions);
  ++_command_count;
}

void command_buffer::copy_image(vk::layer1::image& source_image,
                                vk::image_layout source_layout,
                                vk::layer1::image& destination_image,
                                vk::image_layout destination_layout,
                                const vk::image_copy& copy_region)
{
  vk::cmd_copy_image(_command_buffer, source_image.handle(), source_layout,
                     destination_image.handle(), destination_layout, 1,
                     &copy_region);
  ++_command_count;
}

void command_buffer::blit_image(vk::layer1::image& source_image,
                                vk::image_layout source_layout,
                                vk::layer1::image& destination_image,
                                vk::image_layout destination_layout,
                                const vk::image_blit& blit_region,
                                const vk::filter filter)
{
  vk::cmd_blit_image(_command_buffer, source_image.handle(), source_layout,
                     destination_image.handle(), destination_layout, 1,
                     &blit_region, filter);
  ++_command_count;
}

void command_buffer::copy_buffer_to_image(
  vk::layer1::buffer& source_buffer, vk::layer1::image& destination_image,
  vk::image_layout destination_layout, std::uint32_t buffer_copy_region_count,
  const vk::buffer_image_copy* buffer_copy_regions)
{
  // Copy mip levels from staging buffer
  vk::cmd_copy_buffer_to_image(_command_buffer, source_buffer.handle(),
                               destination_image.handle(), destination_layout,
                               buffer_copy_region_count, buffer_copy_regions);
  ++_command_count;
}

void command_buffer::begin_render_pass(vk::layer1::render_pass& render_pass,
                                       vk::layer1::framebuffer& framebuffer,
                                       const vk::rect_2d& render_area,
                                       std::size_t clear_value_count,
                                       const vk::clear_value* clear_values,
                                       vk::subpass_contents subpass_contents)
{
  vk::cmd_begin_render_pass(
    _command_buffer,
    core::rvalue_address(vk::render_pass_begin_info(
      /* next */ nullptr,
      /* render_pass */ render_pass.handle(),
      /* framebuffer */ framebuffer.handle(),
      /* render_area */ render_area,
      /* clear_value_count */ static_cast<std::uint32_t>(clear_value_count),
      /* clear_values */ clear_values)),
    subpass_contents);
  ++_command_count;
}

void command_buffer::end_render_pass()
{
  vk::cmd_end_render_pass(_command_buffer);
  ++_command_count;
}

void command_buffer::bind_pipeline(vk::pipeline_bind_point bind_point,
                                   vk::layer1::pipeline& pipeline)
{
  vk::cmd_bind_pipeline(_command_buffer, bind_point, pipeline.handle());
  ++_command_count;
}

void command_buffer::bind_descriptor_sets(
  vk::pipeline_bind_point bind_point, const pipeline_layout& layout,
  std::uint32_t first_set, std::uint32_t descriptor_set_count,
  const vk::shared_ptr<vk::layer1::descriptor_set>* descriptor_sets,
  std::uint32_t dynamic_offset_count, const std::uint32_t* dynamic_offsets)
{
  static constexpr std::uint32_t max_descriptor_sets = 4;
  BOOST_ASSERT(descriptor_set_count <= max_descriptor_sets);
  std::array<VkDescriptorSet, max_descriptor_sets> descriptor_set_handles{};
  for (std::uint32_t i = 0; i < descriptor_set_count; ++i, ++descriptor_sets)
  {
    descriptor_set_handles[i] = (*descriptor_sets)->handle();
  }
  vk::cmd_bind_descriptor_sets(_command_buffer, bind_point, layout.handle(),
                               first_set, descriptor_set_count,
                               descriptor_set_handles.data(),
                               dynamic_offset_count, dynamic_offsets);
  ++_command_count;
}

void command_buffer::set_viewports(std::size_t first_viewport,
                                   std::size_t viewport_count,
                                   const vk::viewport* viewports)
{
  vk::cmd_set_viewport(_command_buffer,
                       static_cast<std::uint32_t>(first_viewport),
                       static_cast<std::uint32_t>(viewport_count), viewports);
  ++_command_count;
}

void command_buffer::set_scissors(std::size_t first_scissor,
                                  std::size_t scissor_count,
                                  const vk::rect_2d* scissors)
{
  vk::cmd_set_scissor(_command_buffer,
                      static_cast<std::uint32_t>(first_scissor),
                      static_cast<std::uint32_t>(scissor_count), scissors);
  ++_command_count;
}

void command_buffer::bind_vertex_buffers(
  std::uint32_t first_binding, std::uint32_t binding_count,
  vk::layer1::buffer* const* vertex_buffers, const VkDeviceSize* offsets)
{
  static constexpr std::uint32_t max_vertex_buffers = 8;
  BOOST_ASSERT(binding_count <= max_vertex_buffers);
  std::array<VkBuffer, max_vertex_buffers> buffer_handles{};
  for (std::uint32_t i = 0; i < binding_count; ++i, ++vertex_buffers)
    gsl::at(buffer_handles, i) = (*vertex_buffers)->handle();
  vk::cmd_bind_vertex_buffers(_command_buffer, first_binding, binding_count,
                              buffer_handles.data(), offsets);
  ++_command_count;
}

void command_buffer::bind_index_buffer(vk::layer1::buffer& index_buffer,
                                       VkDeviceSize offset,
                                       vk::index_type index_type)
{
  vk::cmd_bind_index_buffer(_command_buffer, index_buffer.handle(), offset,
                            index_type);
  ++_command_count;
}

void command_buffer::draw(std::uint32_t vertex_count,
                          std::uint32_t instance_count,
                          std::uint32_t first_vertex,
                          std::uint32_t first_instance)
{
  vk::cmd_draw(_command_buffer, vertex_count, instance_count, first_vertex,
               first_instance);
  ++_command_count;
}

void command_buffer::draw_indexed(std::uint32_t index_count,
                                  std::uint32_t instance_count,
                                  std::uint32_t first_index,
                                  std::int32_t vertex_offset,
                                  std::uint32_t first_instance)
{
  vk::cmd_draw_indexed(_command_buffer, index_count, instance_count,
                       first_index, vertex_offset, first_instance);
  ++_command_count;
}

void command_buffer::draw_indirect(vk::layer1::buffer& buffer,
                                   VkDeviceSize offset,
                                   std::uint32_t draw_count,
                                   std::uint32_t stride)
{
  vk::cmd_draw_indirect(_command_buffer, buffer.handle(), offset, draw_count,
                        stride);
  ++_command_count;
}

void command_buffer::draw_indexed_indirect(vk::layer1::buffer& buffer,
                                           VkDeviceSize offset,
                                           std::uint32_t draw_count,
                                           std::uint32_t stride)
{
  vk::cmd_draw_indexed_indirect(_command_buffer, buffer.handle(), offset,
                                draw_count, stride);
  ++_command_count;
}

void command_buffer::dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
  vk::cmd_dispatch(_command_buffer, x, y, z);
  ++_command_count;
}

void command_buffer::dispatch_indirect(vk::layer1::buffer& buffer,
                                       VkDeviceSize offset)
{
  vk::cmd_dispatch_indirect(_command_buffer, buffer.handle(), offset);
  ++_command_count;
}

void command_buffer::execute_commands(
  std::size_t command_buffer_count, vk::layer1::command_buffer* command_buffers)
{
  static constexpr std::uint32_t max_command_buffers = 4;
  BOOST_ASSERT(command_buffer_count <= max_command_buffers);
  std::array<VkCommandBuffer, max_command_buffers> command_buffer_handles{};
  for (std::uint32_t i = 0; i < command_buffer_count; ++i, ++command_buffers)
    gsl::at(command_buffer_handles, i) = command_buffers->handle();
  vk::cmd_execute_commands(_command_buffer,
                           static_cast<std::uint32_t>(command_buffer_count),
                           command_buffer_handles.data());
  ++_command_count;
}

void command_buffer::pipeline_barrier(
  vk::pipeline_stage_flags source_stage_mask,
  vk::pipeline_stage_flags destination_stage_mask,
  vk::dependency_flags dependency_flags, std::size_t memory_barrier_count,
  const vk::memory_barrier* memory_barriers,
  std::size_t buffer_memory_barrier_count,
  const vk::buffer_memory_barrier* buffer_memory_barriers,
  std::size_t image_memory_barrier_count,
  const vk::image_memory_barrier* image_memory_barriers)
{
  vk::cmd_pipeline_barrier(
    _command_buffer, source_stage_mask, destination_stage_mask,
    dependency_flags, static_cast<std::uint32_t>(memory_barrier_count),
    memory_barriers, static_cast<std::uint32_t>(buffer_memory_barrier_count),
    buffer_memory_barriers,
    static_cast<std::uint32_t>(image_memory_barrier_count),
    image_memory_barriers);
  ++_command_count;
}

void command_buffer::resolve_image(vk::layer1::image& source,
                                   vk::image_layout source_layout,
                                   vk::layer1::image& destination,
                                   vk::image_layout destination_layout,
                                   std::uint32_t region_count,
                                   const vk::image_resolve* regions)
{
  vk::cmd_resolve_image(_command_buffer, source.handle(), source_layout,
                        destination.handle(), destination_layout, region_count,
                        regions);
  ++_command_count;
}

void command_buffer::reset_query_pool(vk::layer1::query_pool& pool,
                                      std::uint32_t first_query,
                                      std::uint32_t query_count)
{
  if (query_count == 0)
    query_count = pool.size() - first_query;
  vk::cmd_reset_query_pool(_command_buffer, pool.handle(), first_query,
                           query_count);
  ++_command_count;
}

std::uint32_t command_buffer::begin_query(vk::layer1::query_pool& pool,
                                          vk::query_control_flags flags)
{
  auto query_index = pool.next_index();
  vk::cmd_begin_query(_command_buffer, pool.handle(), query_index, flags);
  ++_command_count;
  return query_index;
}

void command_buffer::end_query(vk::layer1::query_pool& pool,
                               std::uint32_t query_index)
{
  vk::cmd_end_query(_command_buffer, pool.handle(), query_index);
  ++_command_count;
}

void command_buffer::reset_query_pool(vk::layer1::timestamp_query_pool& pool,
                                      std::uint32_t first_query,
                                      std::uint32_t query_count)
{
  if (query_count == 0)
    query_count = pool.size() - first_query;
  vk::cmd_reset_query_pool(_command_buffer, pool.handle(), first_query + 2,
                           query_count * 2);
  ++_command_count;
}

void command_buffer::begin_timestamp(vk::layer1::timestamp_query_pool& pool,
                                     vk::pipeline_stage_flag pipeline_stage,
                                     std::uint32_t query)
{
  vk::cmd_write_timestamp(_command_buffer, pipeline_stage, pool.handle(),
                          query * 2);
  ++_command_count;
}

void command_buffer::end_timestamp(vk::layer1::timestamp_query_pool& pool,
                                   vk::pipeline_stage_flag pipeline_stage,
                                   std::uint32_t query)
{
  vk::cmd_write_timestamp(_command_buffer, pipeline_stage, pool.handle(),
                          query * 2 + 1);
  ++_command_count;
}

void command_buffer::copy_query_results(
  vk::layer1::query_pool& pool, std::uint32_t first_query,
  std::uint32_t query_count, vk::layer1::buffer& destination,
  VkDeviceSize offset, VkDeviceSize stride, vk::query_result_flags flags)
{
  vk::cmd_copy_query_pool_results(_command_buffer, pool.handle(), first_query,
                                  query_count, destination.handle(), offset,
                                  stride, flags);
  ++_command_count;
}
}
