#ifndef SHIFT_RENDER_VK_BUFFER_HPP
#define SHIFT_RENDER_VK_BUFFER_HPP

#include "shift/render/vk/types.hpp"
#include "shift/render/vk/shared_object.hpp"

namespace shift::resource_db
{
class buffer;
}

namespace shift::render::vk
{
///
class buffer : public shared_object
{
public:
  buffer() = default;
  buffer(const buffer&) = delete;
  buffer(buffer&&) = delete;
  ~buffer() noexcept override = 0;
  buffer& operator=(const buffer&) = delete;
  buffer& operator=(buffer&&) = delete;

  /// Returns the size of the buffer in number of bytes.
  virtual std::uint64_t size() noexcept = 0;
};
}

#endif
