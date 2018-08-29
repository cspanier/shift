#ifndef SHIFT_CORE_SINGLETON_H
#define SHIFT_CORE_SINGLETON_H

#include <utility>
#include <memory>
#include "shift/core/core.h"
#include "shift/core/types.h"

namespace shift::core
{
enum class create
{
  using_new,
  on_stack
};

enum class storage
{
  global,
  per_thread
};

namespace detail
{
  template <typename T>
  class thread_specific_ptr
  {
  public:
    thread_specific_ptr() = default;

    explicit thread_specific_ptr(void (*cleanup_function)(T*))
    : _cleanup_function(cleanup_function)
    {
    }

    thread_specific_ptr(const thread_specific_ptr&) = default;
    thread_specific_ptr(thread_specific_ptr&&) noexcept = default;

    ~thread_specific_ptr()
    {
      if (_pointer)
      {
        if (_cleanup_function)
          _cleanup_function(_pointer);
        else
          delete _pointer;
        _pointer = nullptr;
      }
    }

    thread_specific_ptr& operator=(const thread_specific_ptr&) = default;
    thread_specific_ptr& operator=(thread_specific_ptr&&) noexcept = default;

    T* get() const
    {
      return _pointer;
    }

    T* operator->() const
    {
      return _pointer;
    }

    T& operator*() const
    {
      return *_pointer;
    }

    T* release()
    {
      T* result = _pointer;
      _pointer = nullptr;
      return result;
    }

    void reset(T* new_value = nullptr)
    {
      _pointer = new_value;
    }

  private:
    static thread_local T* _pointer = nullptr;
    void (*_cleanup_function)(T*) = nullptr;
  };

  /// A helper type to select the actual storage type depending on the choice
  /// of core::Storage.
  template <typename T, storage Storage>
  struct select_storage;

  template <typename T>
  struct select_storage<T, storage::global>
  {
    using type = std::unique_ptr<T>;
  };

  template <typename T>
  struct select_storage<T, storage::per_thread>
  {
    using type = thread_specific_ptr<T>;
  };
}

template <class T, create create = create::using_new,
          storage Storage = storage::global>
class singleton;

/// A simple singleton implementation enforcing a single instance of type T.
/// The instance is destroyed either on request or automatically on program
/// shutdown.
template <class T, storage Storage>
class singleton<T, create::using_new, Storage>
{
public:
  singleton(const singleton&) = delete;
  singleton& operator=(const singleton&) = delete;
  ~singleton() = default;

  /// Creates the singleton object instance.
  template <typename... Params>
  static inline T& singleton_create(Params&&... params);

  /// Returns the singleton object instance.
  static inline T& singleton_instance();

  /// Returns whether the singleton object has already been instantiated.
  static inline bool singleton_instantiated();

  /// Explicitly destroy the singleton object instance.
  static inline void singleton_destroy();

protected:
  /// Protected default contructor preventing external manual construction.
  singleton() = default;

  singleton(singleton&&) noexcept = default;
  singleton& operator=(singleton&&) noexcept = default;

private:
  /// This type handles automatic destruction of an eventually remaining
  /// instance of T. It is being used within
  /// singleton<T>::singleton_instance().
  class destroy_guard
  {
  public:
    destroy_guard() = default;
    destroy_guard(const destroy_guard&) = default;
    destroy_guard(destroy_guard&&) noexcept = default;
    destroy_guard& operator=(const destroy_guard&) = default;
    destroy_guard& operator=(destroy_guard&&) noexcept = default;

    /// Destructor.
    ~destroy_guard()
    {
      singleton_destroy();
    }
  };

  using storage_type = typename detail::select_storage<T, Storage>::type;

  static storage_type _instance;
};

template <class T, storage Storage>
template <typename... Params>
inline T& singleton<T, create::using_new, Storage>::singleton_create(
  Params&&... params)
{
  // Once code reaches this point we setup an automatic guard, which will
  // destroy the singleton object automatically on shutdown.
  static destroy_guard _guard;

  if (!_instance)
    _instance.reset(new T(std::forward<Params>(params)...));
  return singleton_instance();
}

template <class T, storage Storage>
inline T& singleton<T, create::using_new, Storage>::singleton_instance()
{
  BOOST_ASSERT(_instance);
  return *_instance;
}

template <class T, storage Storage>
inline bool singleton<T, create::using_new, Storage>::singleton_instantiated()
{
  return _instance.get() != nullptr;
}

template <class T, storage Storage>
inline void singleton<T, create::using_new, Storage>::singleton_destroy()
{
  if (_instance)
    _instance.reset();
}

template <class T, storage Storage>
typename singleton<T, create::using_new, Storage>::storage_type
  singleton<T, create::using_new, Storage>::_instance;

/// A simple singleton implementation enforcing a single instance of type T.
/// The instance is managed by the user on the stack and thus automatically
/// destroyed.
template <class T, storage Storage>
class singleton<T, create::on_stack, Storage>
{
public:
  /// Default constructor.
  inline singleton() noexcept;

  singleton(const singleton&) = delete;
  singleton(singleton&&) noexcept = default;

  /// Destructor.
  inline ~singleton();

  singleton& operator=(const singleton&) = delete;
  singleton& operator=(singleton&&) noexcept = default;

  /// Returns the singleton object instance.
  static inline T& singleton_instance();

  /// Returns whether the singleton object has already been instantiated.
  static inline bool singleton_instantiated();

private:
  using storage_type = typename detail::select_storage<T, Storage>::type;

  static storage_type _instance;
};

template <class T, storage Storage>
inline singleton<T, create::on_stack, Storage>::singleton() noexcept
{
  _instance.reset(static_cast<T*>(this));
}

template <class T, storage Storage>
inline singleton<T, create::on_stack, Storage>::~singleton()
{
  _instance.release();
}

template <class T, storage Storage>
inline T& singleton<T, create::on_stack, Storage>::singleton_instance()
{
  BOOST_ASSERT(_instance.get());
  return *_instance;
}

template <class T, storage Storage>
inline bool singleton<T, create::on_stack, Storage>::singleton_instantiated()
{
  return _instance.get() != nullptr;
}

template <class T, storage Storage>
typename singleton<T, create::on_stack, Storage>::storage_type
  singleton<T, create::on_stack, Storage>::_instance;
}

#endif
