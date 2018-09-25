#include "shift/render/vk/renderer_impl.h"
#include "shift/render/vk/window_glfw.h"
#include "shift/render/vk/pass_warp.h"
#include "shift/render/vk/pass_text.h"
#include <shift/render/vk/context_impl.h>
#include <shift/render/vk/geometry_generator.h>
#include "shift/render/vk/layer2/world.h"
#include "shift/render/vk/layer2/camera.h"
#include "shift/render/vk/layer2/texture.h"
#include "shift/render/vk/layer2/view.h"
#include "shift/render/vk/layer2/buffer.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/layer1/instance.h"
#include <shift/resource/repository.h>
#include <shift/resource/shader.h>
#include <shift/log/log.h>

namespace shift::render::vk
{
using namespace std::placeholders;
using namespace std::chrono_literals;

thread_local std::uint32_t renderer_impl::_worker_id =
  std::numeric_limits<decltype(_worker_id)>::max();

std::uint32_t renderer_impl::worker_id() const
{
  BOOST_ASSERT(_worker_id != std::numeric_limits<decltype(_worker_id)>::max());
  return _worker_id;
}

void renderer_impl::async_load(vk::buffer& buffer)
{
  auto new_job = std::make_unique<buffer_load_job>();
  new_job->buffer = &static_cast<vk::layer2::buffer&>(buffer);
  {
    std::scoped_lock lock(_worker_mutex);
    _buffer_load_jobs.emplace(std::move(new_job));
  }
  _worker_condition.notify_one();
}

void renderer_impl::async_load(vk::texture& texture)
{
  auto new_job = std::make_unique<texture_load_job>();
  new_job->texture = &static_cast<vk::layer2::texture&>(texture);

  const auto& source = new_job->texture->source_image();

  // Higher mip levels are stored in a single image buffer for performance
  // reasons. Find the range of collapsed mip levels to load in a single pass.
  auto mip_level_end = new_job->texture->min_valid_mip_level();
  if (mip_level_end == 0u)
  {
    // All mip levels are already loaded.
    return;
  }
  auto mip_level_begin = mip_level_end - 1u;
  while ((mip_level_begin > 0u) &&
         (source->mipmaps[mip_level_end - 1u].buffer.id() ==
          source->mipmaps[mip_level_begin - 1u].buffer.id()))
  {
    --mip_level_begin;
  }
  new_job->mip_level_begin = mip_level_begin;
  new_job->mip_level_end = mip_level_end;
  {
    std::scoped_lock lock(_worker_mutex);
    auto position = std::upper_bound(
      _texture_load_jobs.begin(), _texture_load_jobs.end(), new_job,
      [](const std::unique_ptr<texture_load_job>& lhs,
         const std::unique_ptr<texture_load_job>& rhs) -> bool {
        return lhs->mip_level_end > rhs->mip_level_end;
      });
    _texture_load_jobs.insert(position, std::move(new_job));
  }
  _worker_condition.notify_one();
}

void renderer_impl::process_worker(std::uint32_t worker_id)
{
  _worker_id = worker_id;

  _device->create_worker_resources(worker_id);

  while (!_quit)
  {
    std::unique_ptr<buffer_load_job> buffer_load;
    std::unique_ptr<texture_load_job> texture_load;
    {
      std::unique_lock lock(_worker_mutex);
      _worker_condition.wait(lock, [&]() {
        return _quit ||
               (_active_job_count < config::max_active_job_count &&
                (!_buffer_load_jobs.empty() || !_texture_load_jobs.empty()));
      });
      if (_quit)
        break;
      else if (!_buffer_load_jobs.empty())
      {
        buffer_load = std::move(_buffer_load_jobs.front());
        _buffer_load_jobs.pop();
        ++_active_job_count;
      }
      else if (!_texture_load_jobs.empty())
      {
        texture_load = std::move(_texture_load_jobs.front());
        _texture_load_jobs.pop_front();
        ++_active_job_count;
      }
    }

    if (buffer_load)
    {
      log::debug() << "Async loading buffer " << std::hex
                   << buffer_load->buffer->source_buffer()->id();
      // We can skip this job if nobody except us holds a reference to the
      // buffer any more.
      if (buffer_load->buffer->reference_count() == 1)
        continue;

      buffer_load->buffer->copy_to_staging_buffer();

      // Create transient command_buffer on target device using transfer
      // command_pool.
      auto command_buffer = vk::make_framed_shared<vk::layer1::command_buffer>(
        _device->select_command_pool(vk::layer1::queue_type::transfer, true),
        vk::command_buffer_level::primary);
      command_buffer->begin(vk::command_buffer_begin_info(
        /* next */ nullptr,
        /* flags */ vk::command_buffer_usage_flag::one_time_submit_bit,
        /* inheritance_info */ nullptr));
      // Record command_buffer
      buffer_load->buffer->copy_to_device_memory(*command_buffer);
      command_buffer->end();

      // Create fence object.
      vk::layer1::fence submit_fence(*_device, vk::fence_create_flag::none);
      _device->transfer_queue()->submit(
        vk::submit_info(
          /* next */ nullptr,
          /* wait_semaphore_count */ 0,
          /* wait_semaphores */ nullptr,
          /* wait_dst_stage_mask */ nullptr,
          /* command_buffer_count */ 1,
          /* command_buffers */ &command_buffer->handle(),
          /* signal_semaphore_count */ 0,
          /* signal_semaphores */ nullptr),
        submit_fence.handle());
      // Store buffer, command_buffer, and submit_fence on a separate queue
      // which is periodically checked for finished jobs using submit_fence.
      {
        std::scoped_lock lock(_transfer_mutex);
        _buffer_transfers.emplace(std::move(buffer_load->buffer),
                                  std::move(command_buffer),
                                  std::move(submit_fence));
      }
    }
    else if (texture_load)
    {
      log::debug() << "Async loading texture " << std::hex
                   << texture_load->texture->source_image()->id() << std::dec
                   << " mip levels " << texture_load->mip_level_begin << " to "
                   << texture_load->mip_level_end;
      // We can skip this job if nobody except us holds a reference to the
      // texture any more.
      if (texture_load->texture->reference_count() == 1)
        continue;

      texture_load->texture->copy_to_staging_buffer(
        texture_load->mip_level_begin, texture_load->mip_level_end);

      // Create transient command_buffer on target device using transfer
      // command_pool.
      auto command_buffer = vk::make_framed_shared<vk::layer1::command_buffer>(
        _device->select_command_pool(vk::layer1::queue_type::transfer, true),
        vk::command_buffer_level::primary);
      command_buffer->begin(vk::command_buffer_begin_info(
        /* next */ nullptr,
        /* flags */ vk::command_buffer_usage_flag::one_time_submit_bit,
        /* inheritance_info */ nullptr));
      // Create fence object.
      vk::layer1::fence submit_fence(*_device, vk::fence_create_flag::none);

      // Put mip levels into transfer_dst_optimal layout. We assume that the mip
      // levels were not used before and are still in layout undefined.
      command_buffer->pipeline_barrier(
        vk::pipeline_stage_flag::top_of_pipe_bit,
        vk::pipeline_stage_flag::transfer_bit, vk::dependency_flag::none,
        vk::image_memory_barrier(
          nullptr,
          /* src_access_mask */ vk::access_flag::none,
          /* dst_access_mask */ vk::access_flag::transfer_write_bit,
          /* old_layout */ vk::image_layout::undefined,
          /* new_layout */ vk::image_layout::transfer_dst_optimal,
          /* src_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
          /* dst_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
          /* image */ texture_load->texture->image().handle(),
          /* subresource_range */
          vk::image_subresource_range(
            /* aspect_mask */ vk::image_aspect_flag::color_bit,
            /* base_mip_level */ texture_load->mip_level_begin,
            /* level_count */ texture_load->mip_level_end -
              texture_load->mip_level_begin,
            /* base_array_layer */ 0,
            /* layer_count */ 1)));

      texture_load->texture->copy_to_device_memory(
        *command_buffer, texture_load->mip_level_begin,
        texture_load->mip_level_end);
      command_buffer->end();
      _device->transfer_queue()->submit(
        vk::submit_info(
          /* next */ nullptr,
          /* wait_semaphore_count */ 0,
          /* wait_semaphores */ nullptr,
          /* wait_dst_stage_mask */ nullptr,
          /* command_buffer_count */ 1,
          /* command_buffers */ &command_buffer->handle(),
          /* signal_semaphore_count */ 0,
          /* signal_semaphores */ nullptr),
        submit_fence.handle());
      // Store texture, command_buffer, and submit_fence on a separate queue
      // which is periodically checked for finished jobs using submit_fence.
      {
        std::scoped_lock lock(_transfer_mutex);
        _texture_transfers.emplace(
          std::move(texture_load->texture), std::move(command_buffer),
          std::move(submit_fence), texture_load->mip_level_begin,
          texture_load->mip_level_end);
      }
    }
  }

  _device->destroy_worker_resources(worker_id);
}

void renderer_impl::update_transfer_jobs()
{
  std::size_t finished_jobs = 0;

  // Check if earlier queued texture layout transitions finished.
  while (!_texture_transitions.empty() &&
         _texture_transitions.front().submit_fence.status())
  {
    auto& texture_transition = _texture_transitions.front();
    for (auto& texture : texture_transition.textures)
      texture->availability(vk::layer2::availability_state::loaded);
    /// ToDo: Signal all dependent objects which use the texture.
    log::debug() << "Finished texture transition job.";
    _texture_transitions.pop();
    ++finished_jobs;
  }

  texture_transition_job texture_transition;
  {
    decltype(_buffer_transfers) buffer_transfers;
    decltype(_texture_transfers) texture_transfers;
    {
      std::scoped_lock lock(_transfer_mutex);
      // Find finished transfers in queue by checking for signaled submit_fence
      // objects.
      // log::debug() << "pending texture transfers: " <<
      // _texture_transfers.size();
      _buffer_transfers.swap(buffer_transfers);
      _texture_transfers.swap(texture_transfers);
    }

    while (!buffer_transfers.empty() &&
           buffer_transfers.front().submit_fence.status())
    {
      auto& buffer_transfer = buffer_transfers.front();
      buffer_transfer.buffer->availability(
        vk::layer2::availability_state::loaded);
      /// ToDo: Signal all dependent objects which use the buffer.
      log::debug() << "Finished buffer transfer job.";
      buffer_transfers.pop();
      ++finished_jobs;
    }

    while (!texture_transfers.empty() &&
           texture_transfers.front().submit_fence.status())
    {
      auto& transfer = texture_transfers.front();

      if (!texture_transition.command_buffer)
      {
        texture_transition.command_buffer =
          vk::make_framed_shared<vk::layer1::command_buffer>(
            _device->select_command_pool(vk::layer1::queue_type::graphics,
                                         true),
            vk::command_buffer_level::primary);
        texture_transition.command_buffer->begin(vk::command_buffer_begin_info(
          /* next */ nullptr,
          /* flags */ vk::command_buffer_usage_flag::one_time_submit_bit,
          /* inheritance_info */ nullptr));
      }
      // This cannot be done on a transfer only queue.
      texture_transition.command_buffer->pipeline_barrier(
        vk::pipeline_stage_flag::transfer_bit,
        vk::pipeline_stage_flag::fragment_shader_bit, vk::dependency_flag::none,
        vk::image_memory_barrier(
          /* next */ nullptr,
          /* src_access_mask */ vk::access_flag::transfer_write_bit,
          /* dst_access_mask */ vk::access_flag::shader_read_bit |
            vk::access_flag::input_attachment_read_bit,
          /* old_layout */ vk::image_layout::transfer_dst_optimal,
          /* new_layout */ vk::image_layout::shader_read_only_optimal,
          /* src_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
          /* dst_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
          /* image */ transfer.texture->image().handle(),
          /* subresource_range */
          vk::image_subresource_range(
            /* aspect_mask */ vk::image_aspect_flag::color_bit,
            /* base_mip_level */ transfer.mip_level_begin,
            /* level_count */ transfer.mip_level_end - transfer.mip_level_begin,
            /* base_array_layer */ 0,
            /* layer_count */ transfer.texture->array_layers())));

      transfer.texture->min_valid_mip_level(transfer.mip_level_begin);
      // Each finished texture may start to upload its next mip level.
      if (transfer.texture->reference_count() > 1 &&
          transfer.mip_level_begin > 0)
      {
        async_load(*transfer.texture);
      }
      texture_transition.textures.emplace_back(std::move(transfer.texture));
      log::debug() << "Finished texture transfer job.";
      texture_transfers.pop();
    }

    // Put remaining jobs back on the global queues.
    {
      std::scoped_lock lock(_transfer_mutex);

      _buffer_transfers.swap(buffer_transfers);
      while (!buffer_transfers.empty())
      {
        _buffer_transfers.push(std::move(buffer_transfers.front()));
        buffer_transfers.pop();
      }

      _texture_transfers.swap(texture_transfers);
      while (!texture_transfers.empty())
      {
        _texture_transfers.push(std::move(texture_transfers.front()));
        texture_transfers.pop();
      }
    }
  }
  if (texture_transition.command_buffer)
  {
    texture_transition.command_buffer->end();
    texture_transition.submit_fence =
      vk::layer1::fence(*_device, vk::fence_create_flag::none);
    _device->graphics_queue()->submit(
      vk::submit_info(
        /* next */ nullptr,
        /* wait_semaphore_count */ 0,
        /* wait_semaphores */ nullptr,
        /* wait_dst_stage_mask */ nullptr,
        /* command_buffer_count */ 1,
        /* command_buffers */ &texture_transition.command_buffer->handle(),
        /* signal_semaphore_count */ 0,
        /* signal_semaphores */ nullptr),
      texture_transition.submit_fence.handle());
    // Store the command buffer and all assotiated textures until the commands
    // are fully processed.
    _texture_transitions.emplace(std::move(texture_transition));
  }

  if (finished_jobs > 0)
  {
    {
      std::lock_guard lock(_worker_mutex);
      _active_job_count -= finished_jobs;
    }
    if (finished_jobs == 1)
      _worker_condition.notify_one();
    else
      _worker_condition.notify_all();
  }
}
}
