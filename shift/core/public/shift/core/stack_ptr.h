#ifndef SHIFT_CORE_STACKPTR_H
#define SHIFT_CORE_STACKPTR_H

#include <utility>
#include <type_traits>
#include <shift/platform/assert.h>

namespace shift::core
{
/// This simple class allows dynamic object creation and destruction on
/// preallocated storage.
template <typename T>
class stack_ptr
{
public:
  /// Constructor.
  stack_ptr() noexcept(std::is_nothrow_constructible_v<T>)
  {
  }

  /// Copy constructor.
  stack_ptr(const stack_ptr& other) noexcept(
    std::is_nothrow_copy_constructible_v<T>)
  {
    if (other._constructed)
    {
      new (&_storage) T(*reinterpret_cast<const T*>(&other._storage));
      _constructed = true;
    }
    else
      _constructed = false;
  }

  /// Move constructor.
  stack_ptr(stack_ptr&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    if (other._constructed)
    {
      new (&_storage) T(std::move(*reinterpret_cast<T*>(&other._storage)));
      _constructed = true;
      other._constructed = false;
    }
    else
      _constructed = false;
  }

  /// Destructor.
  ~stack_ptr()
  {
    reset();
  }

  stack_ptr& operator=(const stack_ptr& other) noexcept(
    std::is_nothrow_copy_assignable_v<T>)
  {
    if (other._constructed)
    {
      *reinterpret_cast<T*>(&_storage) =
        *reinterpret_cast<const T*>(&other._storage);
      _constructed = true;
    }
    return *this;
  }

  stack_ptr& operator=(stack_ptr&& other) noexcept(
    std::is_nothrow_move_assignable_v<T>)
  {
    if (other._constructed)
    {
      *reinterpret_cast<T*>(&_storage) =
        std::move(*reinterpret_cast<T*>(&other._storage));
      _constructed = true;
      other._constructed = false;
    }
    else
      _constructed = false;
    return *this;
  }

  stack_ptr& operator=(const T& instance) noexcept(
    std::is_nothrow_copy_assignable_v<T>)
  {
    *reinterpret_cast<T*>(&_storage) = instance;
    _constructed = true;
    return *this;
  }

  stack_ptr& operator=(T&& instance) noexcept(
    std::is_nothrow_move_assignable_v<T>)
  {
    if (!_constructed)
    {
      new (&_storage) T(std::move(instance));
      _constructed = true;
    }
    else
      *reinterpret_cast<T*>(&_storage) = std::move(instance);
    return *this;
  }

  ///
  T& operator*() noexcept
  {
    BOOST_ASSERT(_constructed);
    return *reinterpret_cast<T*>(&_storage);
  }

  ///
  const T& operator*() const noexcept
  {
    BOOST_ASSERT(_constructed);
    return *reinterpret_cast<const T*>(&_storage);
  }

  ///
  T* operator->() noexcept
  {
    BOOST_ASSERT(_constructed);
    return reinterpret_cast<T*>(&_storage);
  }

  ///
  const T* operator->() const noexcept
  {
    BOOST_ASSERT(_constructed);
    return reinterpret_cast<const T*>(&_storage);
  }

  ///
  operator bool() const noexcept
  {
    return _constructed;
  }

  ///
  T* get() noexcept
  {
    if (_constructed)
      return reinterpret_cast<T*>(&_storage);
    else
      return nullptr;
  }

  ///
  const T* get() const noexcept
  {
    if (_constructed)
      return reinterpret_cast<const T*>(&_storage);
    else
      return nullptr;
  }

  ///
  template <typename... Args>
  void create(Args&&... args)
  {
    BOOST_ASSERT(!_constructed);
    if (!_constructed)
    {
      new (&_storage) T(std::forward<Args>(args)...);
      _constructed = true;
    }
  }

  ///
  void reset() noexcept
  {
    if (_constructed)
    {
      reinterpret_cast<T*>(&_storage)->~T();
      _constructed = false;
    }
  }

private:
  union {
    std::aligned_storage_t<sizeof(T), alignof(T)> _storage;
    /// The _object member is used for debugging purposes only.
    T _object;
  };
  bool _constructed = false;
};
}

#endif
