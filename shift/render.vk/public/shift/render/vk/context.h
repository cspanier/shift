#ifndef SHIFT_RENDER_VK_CONTEXT_H
#define SHIFT_RENDER_VK_CONTEXT_H

#include <shift/core/bit_field.h>
#include <shift/math/vector.h>
#include <shift/resource/image.h>
#include "shift/render/vk/types.h"
#include "shift/render/vk/shared_object.h"

namespace shift::render::vk
{
///
class context : public shared_object
{
public:
  context() = default;
  context(const context&) = delete;
  context(context&&) = delete;

  /// Destructor.
  ~context() noexcept override;

  context& operator=(const context&) = delete;
  context& operator=(context&&) = delete;

  ///
  void release() override;

  ///
  virtual bool can_begin() = 0;

  ///
  virtual void begin() = 0;

  ///
  virtual void end() = 0;

  ///
  virtual void wait(std::chrono::nanoseconds timeout) = 0;
};
}

#endif
