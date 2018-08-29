#include <shift/core/types.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <vector>

using namespace shift::core;

BOOST_AUTO_TEST_CASE(types_is_iterator)
{
  using std::begin;
  using std::end;

  BOOST_STATIC_ASSERT(!is_iterator<int>::value);
  BOOST_STATIC_ASSERT(!is_iterator<int[]>::value);
  BOOST_STATIC_ASSERT(!is_iterator<void>::value);

  std::vector<int> vector;
  BOOST_STATIC_ASSERT(is_iterator<decltype(begin(vector))>::value);
  BOOST_STATIC_ASSERT(is_iterator<decltype(end(vector))>::value);

  int c_array[] = {1, 2};
  BOOST_STATIC_ASSERT(is_iterator<decltype(begin(c_array))>::value);
  BOOST_STATIC_ASSERT(is_iterator<decltype(end(c_array))>::value);
}
