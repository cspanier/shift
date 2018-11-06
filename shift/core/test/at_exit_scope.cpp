#include <shift/core/at_exit_scope.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>

using namespace shift::core;

BOOST_AUTO_TEST_CASE(atexitscope_lambda)
{
  bool flag = false;
  {
    auto at_exit = make_at_exit_scope([&]() { flag = true; });
    BOOST_CHECK_EQUAL(flag, false);
  }
  BOOST_CHECK_EQUAL(flag, true);
}

BOOST_AUTO_TEST_CASE(atexitscope_std_function)
{
  bool flag = false;
  std::function<void()> function = [&]() { flag = true; };
  {
    auto at_exit = make_at_exit_scope(function);
    BOOST_CHECK_EQUAL(flag, false);
  }
  BOOST_CHECK_EQUAL(flag, true);
}
