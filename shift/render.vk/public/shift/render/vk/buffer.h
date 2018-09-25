#ifndef SHIFT_RENDER_VK_BUFFER_H
#define SHIFT_RENDER_VK_BUFFER_H

#include "shift/render/vk/types.h"
#include "shift/render/vk/shared_object.h"

namespace shift::resource
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
