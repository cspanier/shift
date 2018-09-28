#ifndef SHIFT_TASK_FUTURE_H
#define SHIFT_TASK_FUTURE_H

#include <shift/core/types.h>
#include <shift/core/exception.h>
#include "shift/task/types.h"
#include "shift/task/mutex.h"
#include "shift/task/condition_variable.h"
#include "shift/task/task_create_info.h"

namespace shift::task
{
///
enum class future_error_code
{
  broken_promise,
  future_already_retrieved,
  promise_already_satisfied,
  no_state
};

/// An exception type thrown on various illegal usage scenarios of future and
/// promise. The exception always has a future_error_code_info attached, and
/// often also a core::context_info.
struct future_error : virtual core::logic_error
{
};

///
using future_error_code_info =
  boost::error_info<struct future_error_code_info_tag, future_error_code>;

namespace detail
{
  ///
  struct private_type
  {
  };

  class associated_state_base
  {
  public:
    std::exception_ptr exception_ptr = nullptr;
    mutex state_mutex;
    condition_variable condition;
    bool already_retrieved = false;
    int ready = 0;
    bool ready_at_task_exit = false;
    bool has_stored_result = false;
    bool running = false;

  protected:
    void queue_task(std::unique_ptr<task_base> new_task);
  };

  ///
  template <typename R>
  class associated_state : public associated_state_base
  {
  public:
    ///
    associated_state() = default;

    associated_state(const associated_state&) = delete;
    associated_state(associated_state&&) = default;
    associated_state& operator=(const associated_state&) = delete;
    associated_state& operator=(associated_state&&) = default;

    ///
    R& value(bool get_only_once)
    {
      std::unique_lock lock(state_mutex);
      if (get_only_once && already_retrieved)
      {
        BOOST_THROW_EXCEPTION(future_error() << future_error_code_info(
                                future_error_code::future_already_retrieved));
      }
      if (exception_ptr)
        std::rethrow_exception(exception_ptr);
      already_retrieved = true;
      // _Maybe_run_deferred_function(_Lock);
      condition.wait(lock, [&] { return ready != 0; });
      if (exception_ptr)
        std::rethrow_exception(exception_ptr);
      return result;
    }

    ///
    void value(const R& new_value, bool defer)
    {
      std::unique_lock lock(state_mutex);
      value(new_value, defer, lock);
    }

    ///
    void value(R&& new_value, bool defer)
    {
      std::unique_lock lock(state_mutex);
      value(std::forward<R>(new_value), defer, lock);
    }

    ///
    void exception(std::exception_ptr new_exception, bool defer)
    {
      std::unique_lock lock(state_mutex);
      exception(new_exception, defer, lock);
    }

    ///
    template <typename Function, typename Future, typename... Args>
    auto then(const task_create_info& create_info, Function&& function,
              Future& future, Args&&... args)
    {
      std::unique_lock lock(state_mutex);
      auto new_task = make_task(create_info, std::forward<Function>(function),
                                future, std::forward<Args>(args)...);
      auto value = new_task->get_future();
      queue_task(std::move(new_task));
      return value;
    }

    ///
    template <typename Function, typename Future, typename... Args>
    auto then(const task_create_info& create_info, Function&& function,
              Future&& future, Args&&... args)
    {
      std::unique_lock lock(state_mutex);
      auto new_task =
        make_task(create_info, std::forward<Function>(function),
                  std::forward<Future>(future), std::forward<Args>(args)...);
      auto value = new_task->get_future();
      queue_task(std::move(new_task));
      return value;
    }

    R result = {};

  private:
    ///
    void value(const R& new_value, bool defer, std::unique_lock<mutex>& lock)
    {
      if (has_stored_result)
      {
        BOOST_THROW_EXCEPTION(future_error() << future_error_code_info(
                                future_error_code::promise_already_satisfied));
      }
      result = new_value;
      notify(lock, defer);
    }

    ///
    void value(R&& new_value, bool defer, std::unique_lock<mutex>& lock)
    {
      if (has_stored_result)
      {
        BOOST_THROW_EXCEPTION(future_error() << future_error_code_info(
                                future_error_code::promise_already_satisfied));
      }
      result = std::forward<R>(new_value);
      notify(lock, defer);
    }

    ///
    void exception(std::exception_ptr new_exception, bool defer,
                   std::unique_lock<mutex>& lock)
    {
      if (has_stored_result)
      {
        BOOST_THROW_EXCEPTION(future_error() << future_error_code_info(
                                future_error_code::promise_already_satisfied));
      }
      exception_ptr =
        new_exception ? new_exception : std::make_exception_ptr(private_type{});
      notify(lock, defer);
    }

    void notify(std::unique_lock<mutex>& lock, bool defer)
    {
      has_stored_result = true;
      if (defer)
        condition.notify_on_task_exit(lock, &ready);
      else
      {
        ready = 1;
        condition.notify_all();
      }
    }
  };

  ///
  template <typename R>
  class state_manager
  {
  public:
    ///
    state_manager() = default;

    ///
    state_manager(std::shared_ptr<associated_state<R>>&& state,
                  bool get_only_once = false)
    : _state(std::move(state)), _get_only_once(get_only_once)
    {
    }

    ///
    state_manager(const detail::state_manager<R>& other,
                  bool get_only_once = false)
    : _state(other._state), _get_only_once(get_only_once)
    {
    }

    ///
    state_manager& operator=(const state_manager& other)
    {
      if (this != &other)
      {
        _state = other._state;
        _get_only_once = other._get_only_once;
      }
      return *this;
    }

    ///
    bool is_valid() const noexcept
    {
      return _state && !(_get_only_once && _state->already_retrieved);
    }

    ///
    R& value() const
    {
      if (!is_valid())
      {
        BOOST_THROW_EXCEPTION(future_error() << future_error_code_info(
                                future_error_code::no_state));
      }
      return _state->value(_get_only_once);
    }

    ///
    void value(const R& new_value, bool defer)
    {
      if (!is_valid())
      {
        BOOST_THROW_EXCEPTION(future_error() << future_error_code_info(
                                future_error_code::no_state));
      }
      _state->value(new_value, defer);
    }

    ///
    void value(R&& new_value, bool defer)
    {
      if (!is_valid())
      {
        BOOST_THROW_EXCEPTION(future_error() << future_error_code_info(
                                future_error_code::no_state));
      }
      _state->value(std::forward<R>(new_value), defer);
    }

    ///
    void exception(std::exception_ptr new_exception, bool defer)
    {
      if (!is_valid())
      {
        BOOST_THROW_EXCEPTION(future_error() << future_error_code_info(
                                future_error_code::no_state));
      }
      _state->exception(new_exception, defer);
    }

    ///
    bool ready()
    {
      if (!is_valid())
      {
        BOOST_THROW_EXCEPTION(future_error() << future_error_code_info(
                                future_error_code::no_state));
      }
      return _state->ready != 0;
    }

  protected:
    std::shared_ptr<associated_state<R>> _state;
    bool _get_only_once = false;
  };
}

///
template <typename R>
class future : protected detail::state_manager<R>
{
public:
  ///
  future() noexcept = default;

  future(const future&) = delete;

  ///
  future(future&& other) noexcept : detail::state_manager<R>(std::move(other))
  {
  }

  future(const detail::state_manager<R>& state, detail::private_type /*unused*/)
  : detail::state_manager<R>(state, true)
  {
  }

  future& operator=(const future&) = delete;

  ///
  future& operator=(future&& other) noexcept
  {
    detail::state_manager<R>::operator=(std::move(other));
    return *this;
  }

  ///
  R get()
  {
    return std::move(detail::state_manager<R>::value());
  }

  ///
  bool ready()
  {
    return detail::state_manager<R>::ready();
  }

  ///
  template <typename Function, typename... Args>
  auto then(const task_create_info& create_info, Function&& function,
            Args&&... args) &
  {
    if (!detail::state_manager<R>::is_valid())
    {
      BOOST_THROW_EXCEPTION(
        future_error() << future_error_code_info(future_error_code::no_state));
    }
    return detail::state_manager<R>::_state->then(
      create_info, std::forward<Function>(function), *this,
      std::forward<Args>(args)...);
  }

  ///
  template <typename Function, typename... Args>
  auto then(const task_create_info& create_info, Function&& function,
            Args&&... args) &&
  {
    if (!detail::state_manager<R>::is_valid())
    {
      BOOST_THROW_EXCEPTION(
        future_error() << future_error_code_info(future_error_code::no_state));
    }
    return detail::state_manager<R>::_state->then(
      create_info, std::forward<Function>(function), std::move(*this),
      std::forward<Args>(args)...);
  }

  ///
  template <
    typename Function, typename... Args,
    DISABLE_IF((std::is_same_v<std::decay_t<Function>, task_create_info>))>
  auto then(Function&& function, Args&&... args) &
  {
    return then(task_create_info{}, std::forward<Function>(function),
                std::forward<Args>(args)...);
  }

  ///
  template <
    typename Function, typename... Args,
    DISABLE_IF((std::is_same_v<std::decay_t<Function>, task_create_info>))>
  auto then(Function&& function, Args&&... args) &&
  {
    return std::move(*this).then(task_create_info{},
                                 std::forward<Function>(function),
                                 std::forward<Args>(args)...);
  }
};

///
template <>
class future<void> : protected detail::state_manager<int>
{
public:
  ///
  future() noexcept = default;

  future(const future&) = delete;

  ///
  future(future&& other) noexcept : detail::state_manager<int>(std::move(other))
  {
  }

  future(const detail::state_manager<int>& state,
         detail::private_type /*unused*/)
  : detail::state_manager<int>(state, true)
  {
  }

  future& operator=(const future&) = delete;

  ///
  future& operator=(future&& other) noexcept
  {
    detail::state_manager<int>::operator=(std::move(other));
    return *this;
  }

  ///
  void get()
  {
    detail::state_manager<int>::value();
  }

  ///
  bool ready()
  {
    return detail::state_manager<int>::ready();
  }

  ///
  template <typename Function, typename... Args>
  auto then(const task_create_info& create_info, Function&& function,
            Args&&... args)
  {
    return detail::state_manager<int>::_state->then(
      create_info, std::forward<Function>(function), std::move(*this),
      std::forward<Args>(args)...);
  }

  ///
  template <
    typename Function, typename... Args,
    DISABLE_IF((std::is_same_v<std::decay_t<Function>, task_create_info>))>
  auto then(Function&& function, Args&&... args)
  {
    return then(task_create_info{}, std::forward<Function>(function),
                std::forward<Args>(args)...);
  }
};

///
template <typename T>
struct future_traits;

template <typename R>
struct future_traits<future<R>>
{
  using value_type = R;
};

///
template <typename T>
using future_traits_t = typename future_traits<T>::value_type;

///
template <typename T>
struct is_future : public std::false_type
{
};

template <typename R>
struct is_future<future<R>> : public std::true_type
{
};
}

#endif
