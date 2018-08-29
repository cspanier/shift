#include "shift/task/async.h"
#include <shift/core/exception.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <thread>
#include <vector>
#include <array>
#include <atomic>
#include <cstdint>

using namespace std::chrono_literals;
using namespace shift;

BOOST_AUTO_TEST_CASE(future_then)
{
  constexpr int result = 42;

  auto first_task = [=]() -> int {
    // This hack should be enough to ensure that then below has been called
    // before this task finishes.
    /// ToDo: Add real thread synchronization to ensure test works.
    std::this_thread::sleep_for(100ms);
    return result;
  };
  auto second_task = [=](task::future<int> first_result) -> int {
    BOOST_CHECK(first_result.ready());
    BOOST_CHECK_EQUAL(first_result.get(), result);
    return 0;
  };
  auto primary_task = [&]() -> int {
    return task::async(first_task).then(second_task).get();
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(future_late_then)
{
  constexpr int result = 42;

  auto primary_task = [&]() {
    auto first_task = [=]() {
      /// ToDo: Add real thread synchronization to ensure test works.
      std::this_thread::sleep_for(100ms);
      return result;
    };
    auto second_task = [=](task::future<int> first_result) {
      BOOST_CHECK(first_result.ready());
      BOOST_CHECK_EQUAL(first_result.get(), result);
      return 0;
    };

    auto first_result = task::async(first_task);
    // This sleep should be enough to ensure that first_task completed before
    // calling then.
    std::this_thread::sleep_for(200ms);
    BOOST_CHECK(first_result.ready());
    return std::move(first_result).then(second_task).get();
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(future_then_chain)
{
  constexpr int result = 42;
  int counter = 0;

  auto primary_task = [&]() {
    auto first_task = [=]() { return 0; };
    auto add_task = [&](task::future<int> /*last_result*/) {
      ++counter;
      return 0;
    };

    auto last_result = task::async(first_task);
    for (int i = 0; i < result; ++i)
    {
      last_result = std::move(last_result).then(add_task);
    }
    return last_result.get();
  };
  task::task_system{}.start(primary_task).join();
  BOOST_CHECK_EQUAL(counter, result);
}

BOOST_AUTO_TEST_CASE(future_when_any_1)
{
  using std::begin;
  using std::end;

  auto primary_task = [&]() {
    std::array<task::future<int>, 2> test_futures = {
      {task::async([]() {
         std::this_thread::sleep_for(100ms);
         return 1;
       }),
       task::async([]() {
         std::this_thread::sleep_for(100ms);
         return 2;
       })}};
    auto result =
      task::when_any(begin(test_futures), end(test_futures))
        .then([](auto futures) {
          BOOST_STATIC_ASSERT(
            std::is_same<decltype(futures),
                         task::future<std::vector<task::future<int>>>>::value);
          for (auto& any_future : futures.get())
          {
            if (any_future.ready())
              return any_future.get();
          }
          throw shift::core::logic_error();
        });
    BOOST_STATIC_ASSERT(
      std::is_same<decltype(result), task::future<int>>::value);
    return result.get();
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(future_when_any_2)
{
  auto primary_task = [&]() {
    auto result =
      task::when_any(task::async([]() {
                       std::this_thread::sleep_for(100ms);
                       return 1;
                     }),
                     task::async([]() {
                       std::this_thread::sleep_for(100ms);
                       return 1.0f;
                     }))
        .then([](auto futures) {
          BOOST_STATIC_ASSERT(
            std::is_same<decltype(futures),
                         task::future<std::tuple<task::future<int>,
                                                 task::future<float>>>>::value);
          auto any_futures = std::move(futures.get());
          if (std::get<0>(any_futures).ready())
            return std::get<0>(any_futures).get();
          if (std::get<1>(any_futures).ready())
            return static_cast<int>(std::get<1>(any_futures).get());
          throw shift::core::logic_error();
        });
    BOOST_STATIC_ASSERT(
      std::is_same<decltype(result), task::future<int>>::value);
    return result.get();
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(future_when_any_empty_1)
{
  using std::begin;
  using std::end;

  auto primary_task = [&]() {
    std::array<task::future<int>, 0> test_futures{};
    auto result =
      task::when_any(begin(test_futures), end(test_futures))
        .then([](auto futures) {
          BOOST_STATIC_ASSERT(
            std::is_same<decltype(futures),
                         task::future<std::vector<task::future<int>>>>::value);
          return 0;
        });
    BOOST_STATIC_ASSERT(
      std::is_same<decltype(result), task::future<int>>::value);
    return result.get();
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(future_when_any_empty_2)
{
  auto primary_task = [&]() {
    auto result = task::when_any().then([](auto futures) {
      BOOST_STATIC_ASSERT(
        std::is_same<decltype(futures), task::future<std::tuple<>>>::value);
      return 0;
    });
    BOOST_STATIC_ASSERT(
      std::is_same<decltype(result), task::future<int>>::value);
    return result.get();
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(future_when_all_1)
{
  using std::begin;
  using std::end;

  auto primary_task = [&]() {
    std::array<task::future<int>, 2> test_futures = {
      {task::async([]() {
         std::this_thread::sleep_for(100ms);
         return 1;
       }),
       task::async([]() {
         std::this_thread::sleep_for(100ms);
         return 2;
       })}};
    auto result =
      task::when_all(begin(test_futures), end(test_futures))
        .then([](auto futures) {
          BOOST_STATIC_ASSERT(
            std::is_same<decltype(futures),
                         task::future<std::vector<task::future<int>>>>::value);
          int sum = 0;
          for (auto& all_future : futures.get())
          {
            BOOST_CHECK(all_future.ready());
            sum += all_future.get();
          }
          BOOST_CHECK_EQUAL(sum, 3);
          return sum;
        });
    BOOST_STATIC_ASSERT(
      std::is_same<decltype(result), task::future<int>>::value);
    return result.get();
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(future_when_all_2)
{
  auto primary_task = [&]() {
    auto result =
      task::when_all(task::async([]() {
                       std::this_thread::sleep_for(100ms);
                       return 1;
                     }),
                     task::async([]() {
                       std::this_thread::sleep_for(100ms);
                       return 1.5f;
                     }))
        .then([](auto futures) {
          BOOST_STATIC_ASSERT(
            std::is_same<decltype(futures),
                         task::future<std::tuple<task::future<int>,
                                                 task::future<float>>>>::value);
          auto all_futures = std::move(futures.get());
          BOOST_CHECK(std::get<0>(all_futures).ready());
          BOOST_CHECK(std::get<1>(all_futures).ready());
          float sum = 0.0f;
          sum += std::get<0>(all_futures).get();
          sum += std::get<1>(all_futures).get();
          BOOST_CHECK_EQUAL(sum, 2.5f);
          return static_cast<int>(sum);
        });
    BOOST_STATIC_ASSERT(
      std::is_same<decltype(result), task::future<int>>::value);
    return result.get();
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(future_when_all_empty_1)
{
  using std::begin;
  using std::end;

  auto primary_task = [&]() {
    std::array<task::future<int>, 0> no_futures{};
    auto result =
      task::when_all(begin(no_futures), end(no_futures)).then([](auto futures) {
        BOOST_STATIC_ASSERT(
          std::is_same<decltype(futures),
                       task::future<std::vector<task::future<int>>>>::value);
        auto empty_vector = std::move(futures.get());
        BOOST_CHECK_EQUAL(empty_vector.size(), 0);
        return empty_vector.size();
      });
    BOOST_STATIC_ASSERT(
      std::is_same<decltype(result), task::future<std::size_t>>::value);
    return static_cast<int>(result.get());
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(future_when_all_empty_2)
{
  auto primary_task = [&]() {
    auto result = task::when_all(/* no futures here */).then([](auto futures) {
      BOOST_STATIC_ASSERT(
        std::is_same<decltype(futures), task::future<std::tuple<>>>::value);
      auto empty_tuple = std::move(futures.get());
      return std::tuple_size_v<decltype(empty_tuple)>;
    });
    BOOST_STATIC_ASSERT(
      std::is_same<decltype(result), task::future<std::size_t>>::value);
    return static_cast<int>(result.get());
  };
  task::task_system{}.start(primary_task).join();
}
