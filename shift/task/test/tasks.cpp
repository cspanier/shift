#include "shift/task/async.hpp"
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <thread>
#include <vector>
#include <random>
#include <atomic>
#include <cstdint>

using namespace std::chrono_literals;
using namespace shift;

BOOST_AUTO_TEST_CASE(test_single_task)
{
  std::atomic<int> counter = ATOMIC_VAR_INIT(0);
  auto primary_task = [&]() {
    ++counter;
    return 0;
  };
  task::task_system{}.start(primary_task).join();
  BOOST_CHECK_EQUAL(counter, 1);
}

BOOST_AUTO_TEST_CASE(test_primary_task_result)
{
  int result = EXIT_FAILURE;

  auto primary_task_success = [&]() { return EXIT_SUCCESS; };
  result = task::task_system{}.start(primary_task_success).join();
  BOOST_CHECK_EQUAL(result, EXIT_SUCCESS);

  auto primary_task_failure = [&]() { return EXIT_FAILURE; };
  result = task::task_system{}.start(primary_task_failure).join();
  BOOST_CHECK_EQUAL(result, EXIT_FAILURE);
}

BOOST_AUTO_TEST_CASE(test_single_task_yield)
{
  std::atomic<int> counter = ATOMIC_VAR_INIT(0);
  auto primary_task = [&]() {
    ++counter;
    task::this_task::yield();
    ++counter;
    return 0;
  };
  task::task_system{}.start(primary_task).join();
  BOOST_CHECK_EQUAL(counter, 2);
}

BOOST_AUTO_TEST_CASE(test_multiple_tasks)
{
  constexpr int result = 42;
  std::atomic<int> counter = ATOMIC_VAR_INIT(0);
  auto secondary_task = [&]() {
    ++counter;
    return result;
  };
  auto primary_task = [&]() {
    ++counter;
    auto r = task::async(secondary_task);
    BOOST_CHECK_EQUAL(r.get(), result);
    return 0;
  };
  task::task_system{}.start(primary_task).join();
  BOOST_CHECK_EQUAL(counter, 2);
}

BOOST_AUTO_TEST_CASE(test_early_exception)
{
  auto secondary_task = [&]() -> bool {
    // The exception will be thrown before r.get() is called below.
    throw std::exception();
  };
  auto primary_task = [&]() {
    bool result = true;
    auto r = task::async(secondary_task);
    std::this_thread::sleep_for(100ms);
    BOOST_CHECK_EXCEPTION(result = r.get(), std::exception,
                          [](const std::exception&) { return true; });
    BOOST_CHECK(result);  // Value of result must not be overwritten.
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(test_late_exception)
{
  auto secondary_task = [&]() -> bool {
    std::this_thread::sleep_for(100ms);
    // The exception will be thrown during r.get() is called below.
    throw std::exception();
  };
  auto primary_task = [&]() {
    bool result = true;
    auto r = task::async(secondary_task);
    BOOST_CHECK_EXCEPTION(result = r.get(), std::exception,
                          [](const std::exception&) { return true; });
    BOOST_CHECK(result);  // Value of result must not be overwritten.
    return 0;
  };
  task::task_system{}.start(primary_task).join();
}

BOOST_AUTO_TEST_CASE(task_worker_affinity)
{
  auto secondary_task = [](task::worker_id_t supposed_worker_id) {
    auto actual_worker_id = task::this_task::current_worker_id();
    auto affinity = task::this_task::worker_affinity();
    BOOST_CHECK_EQUAL(supposed_worker_id, actual_worker_id);
    BOOST_CHECK_EQUAL(affinity, 1u << (actual_worker_id - 1));
    return 0;
  };
  auto primary_task = [&]() {
    auto num_workers = task::task_system::singleton_instance().num_workers();
    std::vector<std::uint32_t> worker_ids;
    worker_ids.reserve(num_workers);
    for (auto worker_id = num_workers; worker_id > 0; --worker_id)
      worker_ids.push_back(worker_id);

    std::random_device random_device;
    std::shuffle(worker_ids.begin(), worker_ids.end(),
                 std::mt19937(random_device()));

    std::vector<task::future<int>> results;
    results.reserve(num_workers);
    for (auto& worker_id : worker_ids)
    {
      results.emplace_back(task::async(
        task::task_create_info{}.worker_affinity(1u << (worker_id - 1)),
        secondary_task, worker_id));
    }
    task::when_all(results.begin(), results.end()).get();

    return 0;
  };
  task::task_system{}.start(primary_task).join();
}
