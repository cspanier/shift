#include <shift/core/at_exit_scope.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

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
