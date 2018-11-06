#ifndef SHIFT_CORE_OPTIONALPTR_HPP
#define SHIFT_CORE_OPTIONALPTR_HPP

#include <memory>
#include <shift/platform/assert.hpp>

namespace shift::core
{
/// A lightweight pointer-wrapper type used to express that an argument may be
/// null.
template <typename T>
class optional_ptr
{
public:
  /// Constructor from a reference enforcing the internal pointer to be
  /// non-nullptr.
  optional_ptr(T& reference) noexcept
  {
    _pointer = &reference;
  }

  /// Constructor from std::nullptr_t setting the internal pointer to nullptr.
  optional_ptr(std::nullptr_t) noexcept
  {
    _pointer = nullptr;
  }

  /// Conversion operator to type T*.
  operator T*() const noexcept
  {
    return _pointer;
  }

  /// Conversion operator to bool, to check whether the internal pointer is
  /// set to a non-null value.
  explicit operator bool() const
  {
    return !!_pointer;
  }

  /// Structure dereference operator.
  T const* operator->() const noexcept
  {
    return _pointer;
  }

  /// Returns the internal pointer to T.
  T* get() noexcept
  {
    return _pointer;
  }

  /// Returns the internal pointer to T.
  const T* get() const noexcept
  {
    return _pointer;
  }

private:
  T* _pointer = nullptr;
};
}

#endif
