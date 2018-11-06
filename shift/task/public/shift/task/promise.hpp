#ifndef SHIFT_TASK_PROMISE_HPP
#define SHIFT_TASK_PROMISE_HPP

#include "shift/task/types.hpp"
#include "shift/task/future.hpp"

namespace shift::task
{
template <typename R>
class promise
{
public:
  /// Default constructor.
  promise() : _state(std::make_shared<detail::associated_state<R>>(), false)
  {
  }

  /// Constructor.
  template <class Alloc>
  promise(std::allocator_arg_t /*unused*/, const Alloc& alloc)
  : _state(std::allocate_shared<detail::associated_state<R>>(alloc), false)
  {
  }

  promise(const promise&) = delete;

  /// Move constructor.
  promise(promise&& other) noexcept
  : _state(std::move(other._state)),
    _future_retrieved(std::move(other._future_retrieved))
  {
  }

  /// Destructor.
  ~promise() = default;

  promise& operator=(const promise&) = delete;

  ///
  promise& operator=(promise&& other) noexcept
  {
    _state = std::move(other._state);
    _future_retrieved = std::move(other._future_retrieved);
    return *this;
  }

  ///
  void swap(promise& /*other*/) noexcept
  {
  }

  future<R> get_future()
  {
    if (_future_retrieved)
    {
      BOOST_THROW_EXCEPTION(
        future_error()
        << future_error_code_info(future_error_code::future_already_retrieved)
        << core::context_info(
             "get_future() has already been called on this promise."));
    }
    _future_retrieved = true;
    return future<R>(_state, detail::private_type{});
  }

  void set_value(const R& new_value)
  {
    if (!_state.is_valid())
    {
      BOOST_THROW_EXCEPTION(
        future_error() << future_error_code_info(future_error_code::no_state));
    }
    _state.value(new_value, false);
  }

  void set_value(R&& new_value)
  {
    if (!_state.is_valid())
    {
      BOOST_THROW_EXCEPTION(
        future_error() << future_error_code_info(future_error_code::no_state));
    }
    _state.value(std::forward<R>(new_value), false);
  }

  // void set_value(R& new_value)
  //{
  //}

  void set_exception(std::exception_ptr new_exception)
  {
    if (!_state.is_valid())
    {
      BOOST_THROW_EXCEPTION(
        future_error() << future_error_code_info(future_error_code::no_state));
    }
    _state.exception(new_exception, false);
  }

private:
  detail::state_manager<R> _state;
  bool _future_retrieved = false;
};

template <>
class promise<void>
{
public:
  /// Default constructor.
  promise() noexcept
  : _state(std::make_shared<detail::associated_state<int>>(), false)
  {
  }

  /// Constructor.
  template <class Alloc>
  promise(std::allocator_arg_t /*unused*/, const Alloc& alloc)
  : _state(std::allocate_shared<detail::associated_state<int>>(alloc), false)
  {
  }

  promise(const promise&) = delete;

  /// Move constructor.
  promise(promise&& other) noexcept
  : _state(std::move(other._state)),
    _future_retrieved(std::move(other._future_retrieved))
  {
  }

  /// Destructor.
  ~promise() = default;

  promise& operator=(const promise&) = delete;

  ///
  promise& operator=(promise&& other) noexcept
  {
    _state = std::move(other._state);
    _future_retrieved = std::move(other._future_retrieved);
    return *this;
  }

  ///
  void swap(promise& /*other*/) noexcept
  {
  }

  future<void> get_future()
  {
    if (_future_retrieved)
    {
      BOOST_THROW_EXCEPTION(
        future_error()
        << future_error_code_info(future_error_code::future_already_retrieved)
        << core::context_info(
             "get_future() has already been called on this promise."));
    }
    _future_retrieved = true;
    return future<void>(_state, detail::private_type{});
  }

  void set_value()
  {
    if (!_state.is_valid())
    {
      BOOST_THROW_EXCEPTION(
        future_error() << future_error_code_info(future_error_code::no_state));
    }
    _state.value(1, false);
  }

  void set_exception(std::exception_ptr new_exception)
  {
    if (!_state.is_valid())
    {
      BOOST_THROW_EXCEPTION(
        future_error() << future_error_code_info(future_error_code::no_state));
    }
    _state.exception(new_exception, false);
  }

private:
  detail::state_manager<int> _state;
  bool _future_retrieved = false;
};
}

#endif
