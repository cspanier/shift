#include <shift/core/optional_ptr.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

BOOST_AUTO_TEST_CASE(optionalptr_null)
{
  auto test = [](shift::core::optional_ptr<int> arg) {
    BOOST_CHECK(!arg);
    BOOST_CHECK(!arg.get());
    BOOST_CHECK(&*arg == nullptr);
    BOOST_CHECK(arg == nullptr);
  };
  test(nullptr);
}

BOOST_AUTO_TEST_CASE(optionalptr_non_null)
{
  auto test = [](shift::core::optional_ptr<int> arg) {
    BOOST_CHECK(arg);
    BOOST_CHECK(arg.get());
    BOOST_CHECK(&*arg != nullptr);
    BOOST_CHECK(arg != nullptr);
    *arg = 42;
  };
  int a = 1;
  test(a);
  BOOST_CHECK_EQUAL(a, 42);
}

BOOST_AUTO_TEST_CASE(optionalptr_const_null)
{
  auto test = [](shift::core::optional_ptr<const int> arg) {
    BOOST_CHECK(!arg);
    BOOST_CHECK(!arg.get());
    BOOST_CHECK(&*arg == nullptr);
    BOOST_CHECK(arg == nullptr);
  };
  test(nullptr);
}

BOOST_AUTO_TEST_CASE(optionalptr_temporary_non_null)
{
  auto test = [](shift::core::optional_ptr<const int> arg) {
    BOOST_CHECK(arg);
    BOOST_CHECK(arg.get());
    BOOST_CHECK(&*arg != nullptr);
    BOOST_CHECK(arg != nullptr);
    BOOST_CHECK_EQUAL(*arg, 5);
  };
  // Const reference parameters allow passing of temporaries.
  test(int{5});
}
