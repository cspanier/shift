#ifndef SHIFT_CORE_RECURSIVEWRAPPER_HPP
#define SHIFT_CORE_RECURSIVEWRAPPER_HPP

#include <memory>
#include <ostream>
#include <shift/platform/assert.hpp>

namespace shift::core
{
/// A wrapper type to solve circular dependencies in type hierarchies.
template <typename T>
class recursive_wrapper
{
public:
  /// Default constructor.
  recursive_wrapper() = default;

  /// Constructor copying a value.
  recursive_wrapper(const T& value) : _data(std::make_unique<T>(value))
  {
  }

  /// Constructor moving a value.
  recursive_wrapper(T&& value)
  : _data(std::make_unique<T>(std::forward<T>(value)))
  {
  }

  /// Copy constructor.
  recursive_wrapper(const recursive_wrapper& other)
  : _data(other._data ? std::make_unique<T>(*other._data) : nullptr)
  {
  }

  /// Move constructor.
  recursive_wrapper(recursive_wrapper&& other) noexcept
  : _data(std::move(other._data))
  {
  }

  /// Destructor.
  ~recursive_wrapper() = default;

  /// Assignment operator from type T, creating a copy of the value.
  recursive_wrapper& operator=(const T& value)
  {
    _data = std::make_unique<T>(value);
  }

  /// Assignment operator from type T, moving the value.
  recursive_wrapper& operator=(T&& value)
  {
    _data = std::make_unique<T>(std::forward<T>(value));
  }

  /// Copy assignment operator.
  recursive_wrapper& operator=(const recursive_wrapper& other)
  {
    if (other._data)
      _data = std::make_unique<T>(*other._data);
    else
      _data.reset();
    return *this;
  }

  /// Move assignment operator.
  recursive_wrapper& operator=(recursive_wrapper&& other) noexcept
  {
    _data = std::move(other._data);
    return *this;
  }

  ///
  inline operator T&()
  {
    BOOST_ASSERT(_data);
    return *_data;
  }

  ///
  inline operator const T&() const
  {
    BOOST_ASSERT(_data);
    return *_data;
  }

  ///
  T* operator->()
  {
    BOOST_ASSERT(_data);
    return _data.get();
  }

  ///
  const T* operator->() const
  {
    BOOST_ASSERT(_data);
    return _data.get();
  }

  ///
  T& operator*()
  {
    BOOST_ASSERT(_data);
    return *_data;
  }

  ///
  const T& operator*() const
  {
    BOOST_ASSERT(_data);
    return *_data;
  }

  ///
  T* get()
  {
    return _data.get();
  }

  ///
  const T* get() const
  {
    return _data.get();
  }

public:
  /// Print the actually stored value.
  friend std::ostream& operator<<(std::ostream& stream,
                                  const recursive_wrapper& value)
  {
    if (value._data)
      stream << *value._data;
    else
      stream << "nullptr";
    return stream;
  }

private:
  std::unique_ptr<T> _data;
};
}

#endif
