#ifndef SHIFT_RENDER_VK_LAYER1_COMMANDBUFFER_H
#define SHIFT_RENDER_VK_LAYER1_COMMANDBUFFER_H

#include <vector>
#include "shift/render/vk/shared.h"
#include "shift/render/vk/layer1/buffer.h"
#include "shift/render/vk/layer1/image.h"
#include "shift/render/vk/layer1/descriptor_set.h"
#include "shift/render/vk/layer1/pipeline.h"
#include "shift/render/vk/layer1/framebuffer.h"
#include "shift/render/vk/layer1/render_pass.h"
#include "shift/render/vk/layer1/query_pool.h"
#include "shift/render/vk/layer1/timestamp_query_pool.h"

namespace shift::render::vk::layer1
{
class device;
class pipeline_layout;
class command_pool;

class command_buffer
{
public:
  /// Constructor.
  command_buffer(
    vk::layer1::command_pool& command_pool,
    vk::command_buffer_level level = vk::command_buffer_level::primary);

  command_buffer(const command_buffer&) = delete;
  command_buffer(command_buffer&&) = delete;

  /// Destructor.
  ~command_buffer();

  command_buffer& operator=(const command_buffer&) = delete;
  command_buffer& operator=(command_buffer&&) = delete;

  ///
  VkCommandBuffer& handle() noexcept
  {
    return _command_buffer;
  }

  ///
  const VkCommandBuffer& handle() const noexcept
  {
    return _command_buffer;
  }

  ///
  bool empty() const noexcept
  {
    return _command_count == 0;
  }

  ///
  void begin(const command_buffer_begin_info& begin_info);

  ///
  void end();

  ///
  void reset(vk::command_buffer_reset_flags reset_flags);

  ///
  void clear_color_image(vk::layer1::image& image,
                         vk::image_layout image_layout,
                         const vk::clear_color_value& color,
                         std::uint32_t range_count,
                         const vk::image_subresource_range* ranges);

  ///
  void clear_depth_stencil_image(
    vk::layer1::image& image, vk::image_layout image_layout,
    const vk::clear_depth_stencil_value& depth_stencil,
    std::uint32_t range_count, const vk::image_subresource_range* ranges);

  ///
  void copy_buffer(vk::layer1::buffer& source_buffer,
                   vk::layer1::buffer& destination_buffer,
                   const std::vector<vk::buffer_copy>& buffer_copy_regions)
  {
    copy_buffer(source_buffer, destination_buffer,
                static_cast<std::uint32_t>(buffer_copy_regions.size()),
                buffer_copy_regions.data());
  }

  ///
  void copy_buffer(vk::layer1::buffer& source_buffer,
                   vk::layer1::buffer& destination_buffer,
                   std::uint32_t buffer_copy_region_count,
                   const vk::buffer_copy* buffer_copy_regions);

  ///
  void copy_image(vk::layer1::image& source_image,
                  vk::image_layout source_layout,
                  vk::layer1::image& destination_image,
                  vk::image_layout destination_layout,
                  const vk::image_copy& copy_region);

  ///
  void blit_image(vk::layer1::image& source_image,
                  vk::image_layout source_layout,
                  vk::layer1::image& destination_image,
                  vk::image_layout destination_layout,
                  const vk::image_blit& blit_region, const vk::filter filter);

  ///
  void copy_buffer_to_image(
    vk::layer1::buffer& source_buffer, vk::layer1::image& destination_image,
    vk::image_layout destination_layout,
    const std::vector<vk::buffer_image_copy>& buffer_copy_regions)
  {
    copy_buffer_to_image(source_buffer, destination_image, destination_layout,
                         static_cast<std::uint32_t>(buffer_copy_regions.size()),
                         buffer_copy_regions.data());
  }

  ///
  void copy_buffer_to_image(vk::layer1::buffer& source_buffer,
                            vk::layer1::image& destination_image,
                            vk::image_layout destination_layout,
                            std::uint32_t buffer_copy_region_count,
                            const vk::buffer_image_copy* buffer_copy_regions);

  ///
  void begin_render_pass(vk::layer1::render_pass& render_pass,
                         vk::layer1::framebuffer& framebuffer,
                         const vk::rect_2d& render_area,
                         std::size_t clear_value_count,
                         const vk::clear_value* clear_values,
                         vk::subpass_contents subpass_contents);

  ///
  void end_render_pass();

  ///
  void bind_pipeline(vk::pipeline_bind_point bind_point,
                     vk::layer1::pipeline& pipeline);

  ///
  void bind_descriptor_sets(
    vk::pipeline_bind_point bind_point, const pipeline_layout& layout,
    std::uint32_t first_set, std::uint32_t descriptor_set_count,
    const vk::shared_ptr<vk::layer1::descriptor_set>* descriptor_sets,
    std::uint32_t dynamic_offset_count, const std::uint32_t* dynamic_offsets);

  ///
  template <std::size_t DescriptorSetCount>
  void bind_descriptor_sets(
    vk::pipeline_bind_point bind_point, const pipeline_layout& layout,
    std::uint32_t first_set,
    const std::array<vk::shared_ptr<vk::layer1::descriptor_set>,
                     DescriptorSetCount>& descriptor_sets)
  {
    bind_descriptor_sets(bind_point, layout, first_set,
                         static_cast<std::uint32_t>(descriptor_sets.size()),
                         descriptor_sets.data(), 0, nullptr);
  }

  ///
  template <std::size_t DescriptorSetHandleCount,
            std::size_t DynamicOffsetCount>
  void bind_descriptor_sets(
    vk::pipeline_bind_point bind_point, const pipeline_layout& layout,
    std::uint32_t first_set,
    const std::array<vk::layer1::descriptor_set&, DescriptorSetHandleCount>&
      descriptor_sets,
    const std::array<std::uint32_t, DynamicOffsetCount>& dynamic_offsets)
  {
    bind_descriptor_sets(bind_point, layout, first_set,
                         static_cast<std::uint32_t>(descriptor_sets.size()),
                         descriptor_sets.data(),
                         static_cast<std::uint32_t>(dynamic_offsets.size()),
                         dynamic_offsets.data());
  }

  ///
  void set_viewports(std::size_t first_viewport, std::size_t viewport_count,
                     const vk::viewport* viewports);

  ///
  template <std::size_t ViewportCount>
  void set_viewports(std::size_t first_viewport,
                     const std::array<vk::viewport, ViewportCount>& viewports)
  {
    set_viewports(first_viewport, viewports.size(), viewports.data());
  }

  ///
  void set_scissors(std::size_t first_scissor, std::size_t scissor_count,
                    const vk::rect_2d* scissors);

  ///
  template <std::size_t ScissorCount>
  void set_scissors(std::size_t first_scissor,
                    const std::array<vk::rect_2d, ScissorCount>& scissors)
  {
    set_scissors(first_scissor, scissors.size(), scissors.data());
  }

  ///
  void bind_vertex_buffers(std::uint32_t first_binding,
                           std::uint32_t binding_count,
                           vk::layer1::buffer* const* vertex_buffers,
                           const VkDeviceSize* offsets);

  ///
  template <std::size_t BufferCount>
  void bind_vertex_buffers(
    std::uint32_t first_binding,
    const std::array<vk::layer1::buffer*, BufferCount>& vertex_buffers,
    const std::array<VkDeviceSize, BufferCount>& offsets)
  {
    bind_vertex_buffers(first_binding, BufferCount, vertex_buffers.data(),
                        offsets.data());
  }

  ///
  void bind_index_buffer(vk::layer1::buffer& index_buffer, VkDeviceSize offset,
                         vk::index_type index_type);

  ///
  void draw(std::uint32_t vertex_count, std::uint32_t instance_count,
            std::uint32_t first_vertex, std::uint32_t first_instance);

  ///
  void draw_indexed(std::uint32_t index_count, std::uint32_t instance_count,
                    std::uint32_t first_index, std::int32_t vertex_offset,
                    std::uint32_t first_instance);

  ///
  void draw_indirect(vk::layer1::buffer& buffer, VkDeviceSize offset,
                     std::uint32_t draw_count, std::uint32_t stride);

  ///
  void draw_indexed_indirect(vk::layer1::buffer& buffer, VkDeviceSize offset,
                             std::uint32_t draw_count, std::uint32_t stride);

  ///
  void dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z);

  ///
  void dispatch_indirect(vk::layer1::buffer& buffer, VkDeviceSize offset);

  ///
  void execute_commands(std::size_t command_buffer_count,
                        vk::layer1::command_buffer* command_buffers);

  ///
  template <std::size_t CommandBufferCount>
  void execute_commands(
    const std::array<vk::layer1::command_buffer&, CommandBufferCount>
      command_buffers)
  {
    execute_commands(static_cast<std::uint32_t>(command_buffers.size()),
                     command_buffers.data());
  }

  ///
  void execute_commands(vk::layer1::command_buffer& command_buffer)
  {
    execute_commands(1, &command_buffer);
  }

  ///
  void pipeline_barrier(vk::pipeline_stage_flags source_stage_mask,
                        vk::pipeline_stage_flags destination_stage_mask,
                        vk::dependency_flags dependency_flags,
                        std::size_t memory_barrier_count,
                        const vk::memory_barrier* memory_barriers,
                        std::size_t buffer_memory_barrier_count,
                        const vk::buffer_memory_barrier* buffer_memory_barriers,
                        std::size_t image_memory_barrier_count,
                        const vk::image_memory_barrier* image_memory_barriers);

  ///
  void pipeline_barrier(vk::pipeline_stage_flags source_stage_mask,
                        vk::pipeline_stage_flags destination_stage_mask,
                        vk::dependency_flags dependency_flags,
                        const vk::memory_barrier& memory_barrier)
  {
    pipeline_barrier(source_stage_mask, destination_stage_mask,
                     dependency_flags, 1, &memory_barrier, 0, nullptr, 0,
                     nullptr);
  }

  ///
  void pipeline_barrier(vk::pipeline_stage_flags source_stage_mask,
                        vk::pipeline_stage_flags destination_stage_mask,
                        vk::dependency_flags dependency_flags,
                        const vk::buffer_memory_barrier& buffer_memory_barrier)
  {
    pipeline_barrier(source_stage_mask, destination_stage_mask,
                     dependency_flags, 0, nullptr, 1, &buffer_memory_barrier, 0,
                     nullptr);
  }

  ///
  void pipeline_barrier(vk::pipeline_stage_flags source_stage_mask,
                        vk::pipeline_stage_flags destination_stage_mask,
                        vk::dependency_flags dependency_flags,
                        const vk::image_memory_barrier& image_memory_barrier)
  {
    pipeline_barrier(source_stage_mask, destination_stage_mask,
                     dependency_flags, 0, nullptr, 0, nullptr, 1,
                     &image_memory_barrier);
  }

  ///
  template <std::size_t MemoryBarrierCount,
            std::size_t BufferMemoryBarrierCount,
            std::size_t ImageMemoryBarrierCount>
  void pipeline_barrier(
    vk::pipeline_stage_flags source_stage_mask,
    vk::pipeline_stage_flags destination_stage_mask,
    vk::dependency_flags dependency_flags,
    const std::array<vk::memory_barrier, MemoryBarrierCount>& memory_barriers,
    const std::array<vk::buffer_memory_barrier, BufferMemoryBarrierCount>&
      buffer_memory_barriers,
    const std::array<vk::image_memory_barrier, ImageMemoryBarrierCount>&
      image_memory_barriers)
  {
    pipeline_barrier(
      source_stage_mask, destination_stage_mask, dependency_flags,
      static_cast<std::uint32_t>(MemoryBarrierCount), memory_barriers.data(),
      static_cast<std::uint32_t>(BufferMemoryBarrierCount),
      buffer_memory_barriers.data(),
      static_cast<std::uint32_t>(ImageMemoryBarrierCount),
      image_memory_barriers.data());
  }

  ///
  void resolve_image(vk::layer1::image& source, vk::image_layout source_layout,
                     vk::layer1::image& destination,
                     vk::image_layout destination_layout,
                     std::uint32_t region_count,
                     const vk::image_resolve* regions);

  ///
  void resolve_image(vk::layer1::image& source, vk::image_layout source_layout,
                     vk::layer1::image& destination,
                     vk::image_layout destination_layout,
                     const vk::image_resolve& region)
  {
    resolve_image(source, source_layout, destination, destination_layout, 1,
                  &region);
  }

  ///
  void reset_query_pool(vk::layer1::query_pool& pool,
                        std::uint32_t first_query = 0,
                        std::uint32_t query_count = 0);

  ///
  std::uint32_t begin_query(vk::layer1::query_pool& pool,
                            vk::query_control_flags flags);

  ///
  void end_query(vk::layer1::query_pool& pool, std::uint32_t query_index);

  ///
  void reset_query_pool(vk::layer1::timestamp_query_pool& pool,
                        std::uint32_t first_query = 0,
                        std::uint32_t query_count = 0);

  ///
  void begin_timestamp(vk::layer1::timestamp_query_pool& pool,
                       vk::pipeline_stage_flag pipeline_stage,
                       std::uint32_t query);

  ///
  void end_timestamp(vk::layer1::timestamp_query_pool& pool,
                     vk::pipeline_stage_flag pipeline_stage,
                     std::uint32_t query);

  ///
  void copy_query_results(vk::layer1::query_pool& pool,
                          std::uint32_t first_query, std::uint32_t query_count,
                          vk::layer1::buffer& destination, VkDeviceSize offset,
                          VkDeviceSize stride, vk::query_result_flags flags);

private:
  command_pool* _command_pool = nullptr;
  VkCommandBuffer _command_buffer = nullptr;
  std::uint32_t _command_count = 0;
};
}

#endif
