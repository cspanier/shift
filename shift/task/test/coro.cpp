#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <boost/coroutine2/coroutine.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <cstdint>
#if defined(SHIFT_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

using coro_t = boost::coroutines2::coroutine<int>;

BOOST_AUTO_TEST_CASE(test_windows_boost_bug)
{
  bool result = false;

  auto coro_function = [&](coro_t::push_type& /*sink*/) {
#if defined(SHIFT_PLATFORM_WINDOWS)
    char buffer[MAX_PATH];
    // This simple Windows API call crashes when using MSVC2015 on Windows in
    // debug build only. The reason seems to be an uninitialized stack memory,
    // and thus invalid function pointers on the call stack.
    GetModuleFileName(nullptr, buffer, MAX_PATH);
#endif
    result = true;
  };

  coro_t::pull_type{coro_function};
  BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE(test_coro_thread_migration)
{
  std::unique_ptr<coro_t::pull_type> coro;

  auto coro_function = [](coro_t::push_type& sink) {
    sink(1);
    sink(1);
    sink(2);
    sink(3);
    sink(5);
    sink(8);
  };

  std::mutex m;

  auto thread_function = [&]() {
    using namespace std::chrono_literals;
    for (;;)
    {
      std::this_thread::sleep_for(100ms);
      std::unique_lock lock(m);
      if (!coro)
        coro = std::make_unique<coro_t::pull_type>(coro_function);
      else
        (*coro)();
      if (!*coro)
      {
        coro.reset();
        break;
      }
    }
  };

  std::thread t1(thread_function);
  std::thread t2(thread_function);
  t1.join();
  t2.join();
}
