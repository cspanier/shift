#ifndef SHIFT_TASK_TASK_HPP
#define SHIFT_TASK_TASK_HPP

#include <memory>
#include <functional>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/coroutine2/detail/forced_unwind.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/task/types.hpp"
#include "shift/task/promise.hpp"
#include "shift/task/task_create_info.hpp"
#include "shift/task/this_task.hpp"

namespace shift::task
{
/// Base class for all tasks.
class task_base
{
public:
  ///
  task_base(const task_create_info& create_info,
            std::function<void()> wrapped_function);

  task_base(const task_base&) = delete;
  task_base(task_base&&) = delete;

  /// Destructor.
  virtual ~task_base();

  task_base& operator=(const task_base&) = delete;
  task_base& operator=(task_base&&) = delete;

  ///
  void yield();

  ///
  task_id_t id() const;

  /// Forces the task to be processed only by specific worker threads.
  void worker_affinity(worker_affinity_t affinity);

  /// Returns the worker affinity mask.
  worker_affinity_t worker_affinity() const;

private:
  friend class task_system;

  struct impl;
  std::unique_ptr<impl> _impl;
};

template <typename Result>
class task_result : public task_base
{
public:
  using result_t = Result;

  ///
  task_result(const task_create_info& create_info,
              std::function<void()> wrapped_function)
  : task_base(create_info, std::move(wrapped_function))
  {
  }

  ///
  auto get_future()
  {
    return _promise.get_future();
  }

protected:
  promise<result_t> _promise;
};

/// Explicit task providing a promise about the result of the task function.
template <typename Function, typename... Args>
class task
: public task_result<std::result_of_t<std::decay_t<Function>(Args...)>>
{
public:
  using base_t = task_result<std::result_of_t<std::decay_t<Function>(Args...)>>;

  /// Constructor.
  task(const task_create_info& create_info, Function&& function, Args&&... args)
  : base_t(create_info,
           [&]() {
             try
             {
               base_t::_promise.set_value(
                 call_function(_function, std::index_sequence_for<Args...>{}));
             }
             catch (const boost::coroutines2::detail::forced_unwind&)
             {
               // Ignore this type of exception.
             }
             catch (...)
             {
               base_t::_promise.set_exception(std::current_exception());
             }
           }),
    _function(std::forward<Function>(function)),
    _args(std::move(args)...)
  {
  }

  /// Destructor.
  ~task() override = default;

private:
  ///
  template <std::size_t... Is>
  typename base_t::result_t call_function(Function function,
                                          std::index_sequence<Is...> /*unused*/)
  {
    return function(std::forward<std::tuple_element_t<Is, std::tuple<Args...>>>(
      std::get<Is>(_args))...);
  }

  Function _function;
  std::tuple<std::remove_reference_t<Args>...> _args;
};
}

#endif
