#ifndef SHIFT_TASK_WHEN_ANY_HPP
#define SHIFT_TASK_WHEN_ANY_HPP

#include <atomic>
#include "shift/task/mutex.hpp"
#include "shift/task/promise.hpp"
#include "shift/task/future.hpp"

namespace shift::task
{
namespace detail
{
  template <typename Futures>
  struct when_any_data
  {
    std::atomic_flag satisfied = ATOMIC_FLAG_INIT;
    promise<Futures> promised_result;
    mutex futures_mutex;
    Futures futures;
  };
}

///
template <typename Iterator>
auto when_any(Iterator begin, Iterator end,
              ENABLE_IF(core::is_iterator<Iterator>::value))
{
  using future_vector =
    std::vector<typename std::iterator_traits<Iterator>::value_type>;

  // Create a shared data structure, mainly used to pass a single promise to
  // several helper tasks.
  auto shared_data = std::make_shared<detail::when_any_data<future_vector>>();
  // Handle a special case where we need to manually satisfy the promise
  // because no one else would.
  if (begin == end)
  {
    shared_data->promised_result.set_value(std::move(shared_data->futures));
    return shared_data->promised_result.get_future();
  }
  // Synchronize access to shared_data->futures, because it could be moved
  // from within the inner lambda.
  std::unique_lock futures_lock(shared_data->futures_mutex);
  // Move all passed futures from [begin..end) to a separate storage.
  shared_data->futures.reserve(std::distance(begin, end));
  for (auto future_iter = begin; future_iter != end; ++future_iter)
    shared_data->futures.emplace_back(std::move(*future_iter));
  // If any of the passed futures get available, satisfy the shared promise.
  for (auto& future_iter : shared_data->futures)
  {
    future_iter.then([shared_data](auto& /*any_future*/) {
      // Only the first available future may satisfy the promise.
      if (!shared_data->satisfied.test_and_set())
      {
        // Synchronize access to shared_data->futures, because the outer for
        // loop might still be iterating through the list.
        std::lock_guard futures_lock(shared_data->futures_mutex);
        shared_data->promised_result.set_value(std::move(shared_data->futures));
      }
      /// ToDo: We don't want to return anything here (requires future<void>).
      return true;
    });
  }
  // Unlock futures_mutex befor waiting on the result.
  futures_lock.unlock();
  // Return the promised future object.
  return shared_data->promised_result.get_future();
}

///
template <typename... Futures>
auto when_any(Futures&&... futures)
{
  using future_tuple = std::tuple<Futures...>;

  // Create a shared data structure, mainly used to pass a single promise to
  // several helper tasks.
  auto shared_data = std::make_shared<detail::when_any_data<future_tuple>>();
  // Handle a special case where we need to manually satisfy the promise
  // because no one else would.
  if constexpr (sizeof...(Futures) == 0)
  {
    shared_data->promised_result.set_value(std::move(shared_data->futures));
    return shared_data->promised_result.get_future();
  }

  auto wait_on_future = [&shared_data](auto& any_future) {
    any_future.then([shared_data](auto& /* any_future */) {
      // Only the first available future may satisfy the promise.
      if (!shared_data->satisfied.test_and_set())
      {
        // Synchronize access to shared_data->futures,
        // because the outer for loop might still be
        // iterating through the list.
        std::lock_guard futures_lock(shared_data->futures_mutex);
        shared_data->promised_result.set_value(std::move(shared_data->futures));
      }
      /// ToDo: We don't want to return anything here
      /// (requires future<void>).
      return true;
    });
  };

  {
    // Synchronize access to shared_data->futures, because it could be moved
    // from within the for_each_element lambda.
    std::lock_guard futures_lock(shared_data->futures_mutex);
    // Move all passed futures to a separate storage.
    shared_data->futures = std::make_tuple(std::forward<Futures>(futures)...);
    // If any of the passed futures get available, satisfy the shared promise.
    core::for_each_element(shared_data->futures, wait_on_future);
    // Unlock futures_mutex befor waiting on the result.
  }
  // Return the promised future object.
  return shared_data->promised_result.get_future();
}
}

#endif
