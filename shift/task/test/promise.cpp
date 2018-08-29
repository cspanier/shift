#include "shift/task/async.h"
#include <shift/core/exception.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <thread>
#include <vector>
#include <atomic>
#include <cstdint>

using namespace std::chrono_literals;
using namespace shift;

BOOST_AUTO_TEST_CASE(promise_inactive)
{
  auto primary_task = [&]() {
    task::promise<int> p;
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(promise_active_unused)
{
  auto primary_task = [&]() {
    task::promise<int> p;
    auto f = p.get_future();
    BOOST_CHECK(!f.ready());
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(promise_multiple_futures)
{
  auto primary_task = [&]() {
    task::promise<int> p;
    auto f = p.get_future();
    BOOST_CHECK(!f.ready());
    BOOST_CHECK_EXCEPTION(
      p.get_future(), task::future_error,
      [](const task::future_error& /*ex*/) { return true; });
    BOOST_CHECK(!f.ready());
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(promise_satisfied_1)
{
  constexpr int result = 1;

  auto primary_task = [&]() {
    task::promise<int> p;
    p.set_value(result);
    auto f = p.get_future();
    BOOST_CHECK(f.ready());
    BOOST_CHECK_EQUAL(f.get(), result);
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(promise_satisfied_2)
{
  constexpr int result = 2;

  auto primary_task = [&]() {
    task::promise<int> p;
    auto f = p.get_future();
    BOOST_CHECK(!f.ready());
    p.set_value(result);
    BOOST_CHECK(f.ready());
    BOOST_CHECK_EQUAL(f.get(), result);
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(promise_satisfied_3)
{
  constexpr int result = 3;

  auto primary_task = [&]() {
    task::promise<int> p;
    {
      task::promise<int> p2(std::move(p));
      p2.set_value(result);
    }
    auto f = p.get_future();
    BOOST_CHECK(f.ready());
    BOOST_CHECK_EQUAL(f.get(), result);
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(promise_satisfied_4)
{
  constexpr int result = 4;

  auto primary_task = [&]() {
    task::promise<int> p;
    auto f = p.get_future();
    BOOST_CHECK(!f.ready());
    {
      task::promise<int> p2(std::move(p));
      p2.set_value(result);
    }
    BOOST_CHECK(f.ready());
    BOOST_CHECK_EQUAL(f.get(), result);
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(promise_already_satisfied)
{
  constexpr int result = 42;

  auto primary_task = [&]() {
    task::promise<int> p;
    auto f = p.get_future();
    BOOST_CHECK(!f.ready());

    p.set_value(result);
    BOOST_CHECK(f.ready());
    BOOST_CHECK_EXCEPTION(
      p.set_value(result), task::future_error,
      [](const task::future_error& /*ex*/) { return true; });
    BOOST_CHECK(f.ready());
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(promise_exception)
{
  auto primary_task = [&]() {
    task::promise<int> p;
    auto f = p.get_future();
    BOOST_CHECK(!f.ready());

    {
      task::promise<int> p2(std::move(p));
      try
      {
        BOOST_THROW_EXCEPTION(core::logic_error());
      }
      catch (...)
      {
        p2.set_exception(std::current_exception());
      }
    }

    BOOST_CHECK(f.ready());
    BOOST_CHECK_EXCEPTION(f.get(), core::logic_error,
                          [](const core::logic_error& /*ex*/) { return true; });
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(promise_block_for_satisfied)
{
  constexpr int result = 2;

  auto secondary_task = [&](task::promise<int> p) {
    // We should not send the worker thread that processes this task to sleep,
    // but it is ok in the context of this unit test.
    std::this_thread::sleep_for(100ms);
    p.set_value(result);
    return 0;
  };
  auto primary_task = [&]() {
    task::promise<int> p;
    auto f = p.get_future();
    async({}, secondary_task, std::move(p));
    BOOST_CHECK_EQUAL(f.get(), result);
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}
