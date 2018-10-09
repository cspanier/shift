#ifndef SHIFT_RENDER_VK_SHARED_OBJECT_H
#define SHIFT_RENDER_VK_SHARED_OBJECT_H

#include <cstdint>
#include <atomic>
#include <shift/core/boost_disable_warnings.h>
#include <boost/intrusive_ptr.hpp>
#include <shift/core/boost_restore_warnings.h>

namespace shift::render::vk
{
///
class shared_object
{
public:
  shared_object() = default;
  shared_object(const shared_object&) = delete;
  shared_object(const shared_object&&) = delete;
  virtual ~shared_object() noexcept = 0;
  shared_object& operator=(const shared_object&) = delete;
  shared_object& operator=(shared_object&&) = delete;

  /// Claims shared ownership of this object.
  void add_reference();

  /// Releases shared ownership of this object.
  virtual void release();

  /// Returns the number of references to this object.
  std::uint32_t reference_count() const;

protected:
  /// Initialize reference counter with one and save an atomic increment on
  /// boost::intrusive_ptr construction.
  std::atomic<std::uint32_t> _reference_counter = ATOMIC_VAR_INIT(1u);
};

/// Function used by boost::intrusive_ptr to increment reference count.
inline void intrusive_ptr_add_ref(shared_object* object)
{
  object->add_reference();
}

/// Function used by boost::intrusive_ptr to decrement reference count.
inline void intrusive_ptr_release(shared_object* object)
{
  object->release();
}
}

#endif
