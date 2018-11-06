#include "shift/render/vk/shared_object.hpp"

namespace shift::render::vk
{
shared_object::~shared_object() noexcept = default;

void shared_object::add_reference()
{
  _reference_counter.fetch_add(1, std::memory_order_relaxed);
}

void shared_object::release()
{
  if (_reference_counter.fetch_sub(1, std::memory_order_release) == 1)
  {
    std::atomic_thread_fence(std::memory_order_acquire);
    delete this;
  }
}

std::uint32_t shared_object::reference_count() const
{
  return _reference_counter;
}
}
