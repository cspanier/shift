#ifndef SHIFT_RENDER_VK_LAYER2_BUFFER_HPP
#define SHIFT_RENDER_VK_LAYER2_BUFFER_HPP

#include <memory>
#include "shift/render/vk/types.hpp"
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/smart_ptr.hpp"
#include "shift/render/vk/buffer.hpp"
#include "shift/render/vk/layer1/buffer.hpp"
#include "shift/render/vk/layer2/object.hpp"

namespace shift::resource_db
{
class buffer;
}

namespace shift::render::vk::layer1
{
class device;
class command_buffer;
}

namespace shift::render::vk::layer2
{
///
enum class buffer_state
{
  unavailable,
  queued,
  uploading,
  available
};

///
class buffer final : public vk::buffer, public child_object
{
public:
  /// Constructor.
  /// @param source
  ///   An optional data source.
  buffer(vk::layer1::device& device, std::uint64_t size,
         vk::buffer_usage_flags usage,
         std::shared_ptr<resource_db::buffer> source);

  buffer(const buffer&) = delete;
  buffer(buffer&&) = delete;
  ~buffer() noexcept final;
  buffer& operator=(const buffer&) = delete;
  buffer& operator=(buffer&&) = delete;

  ///
  const std::shared_ptr<resource_db::buffer>& source_buffer() const;

  ///
  std::uint64_t size() noexcept final;

  /// Copies the buffer content from the assigned buffer resource to the staging
  /// buffer.
  /// @pre
  ///   The buffer has been assigned a valid buffer resource at construction.
  void copy_to_staging_buffer();

  /// Copies the staging buffer to device memory.
  /// @pre
  ///   All data in the staging buffer has been loaded using
  ///   copy_to_staging_buffer.
  void copy_to_device_memory(vk::layer1::command_buffer& command_buffer);

private:
  vk::layer1::device* _device = nullptr;
  vk::shared_ptr<vk::layer1::buffer> _buffer;
  vk::shared_ptr<vk::layer1::buffer> _staging_buffer;
  std::uint64_t _size = 0;
  vk::buffer_usage_flags _usage = vk::buffer_usage_flag::none;
  std::shared_ptr<resource_db::buffer> _source;
};
}

#endif
