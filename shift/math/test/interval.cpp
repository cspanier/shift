#include <shift/math/interval.hpp>
#include <shift/core/algorithm.hpp>
#include <shift/platform/fpexceptions.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>

#include <thread>

using namespace shift;
using namespace shift::math;

BOOST_AUTO_TEST_CASE(interval_is_trivial)
{
  static_assert(std::is_trivial_v<interval<2, int>>);
  static_assert(std::is_trivial_v<interval<2, float>>);
  BOOST_CHECK((std::is_trivial_v<interval<2, int>>));
  BOOST_CHECK((std::is_trivial_v<interval<2, float>>));
}

BOOST_AUTO_TEST_CASE(interval_make_from)
{
  platform::floating_point_exceptions fpexceptions;

  auto v1 = make_vector_from(1.0f, 2.0f);
  auto v2 = make_vector_from(3.0f, 6.0f);
  auto v3 = make_vector_from(5.0f, 4.0f);

  auto r1 = make_interval_from(v1);
  BOOST_CHECK_EQUAL(r1.min.x, 1.0f);
  BOOST_CHECK_EQUAL(r1.min.y, 2.0f);
  BOOST_CHECK_EQUAL(r1.max.x, 1.0f);
  BOOST_CHECK_EQUAL(r1.max.y, 2.0f);

  auto r2 = make_interval_from(v1, v2);
  BOOST_CHECK_EQUAL(r2.min.x, 1.0f);
  BOOST_CHECK_EQUAL(r2.min.y, 2.0f);
  BOOST_CHECK_EQUAL(r2.max.x, 3.0f);
  BOOST_CHECK_EQUAL(r2.max.y, 6.0f);

  auto r3 = make_interval_from(v1, v2, v3);
  BOOST_CHECK_EQUAL(r3.min.x, 1.0f);
  BOOST_CHECK_EQUAL(r3.min.y, 2.0f);
  BOOST_CHECK_EQUAL(r3.max.x, 5.0f);
  BOOST_CHECK_EQUAL(r3.max.y, 6.0f);
}

BOOST_AUTO_TEST_CASE(interval_make_union)
{
  platform::floating_point_exceptions fpexceptions;

  interval<2, float> a(make_vector_from(1.0f, 2.0f),
                       make_vector_from(4.0f, 3.0f));
  interval<2, float> b(make_vector_from(2.0f, 1.0f),
                       make_vector_from(3.0f, 4.0f));
  interval<2, float> c(make_vector_from(6.0f, 7.0f),
                       make_vector_from(8.0f, 9.0f));
  auto r1 = make_interval_union(a);
  BOOST_CHECK_EQUAL(r1.min.x, 1.0f);
  BOOST_CHECK_EQUAL(r1.min.y, 2.0f);
  BOOST_CHECK_EQUAL(r1.max.x, 4.0f);
  BOOST_CHECK_EQUAL(r1.max.y, 3.0f);

  auto r2 = make_interval_union(a, b);
  BOOST_CHECK_EQUAL(r2.min.x, 1.0f);
  BOOST_CHECK_EQUAL(r2.min.y, 1.0f);
  BOOST_CHECK_EQUAL(r2.max.x, 4.0f);
  BOOST_CHECK_EQUAL(r2.max.y, 4.0f);

  auto r3 = make_interval_union(a, b, c);
  BOOST_CHECK_EQUAL(r3.min.x, 1.0f);
  BOOST_CHECK_EQUAL(r3.min.y, 1.0f);
  BOOST_CHECK_EQUAL(r3.max.x, 8.0f);
  BOOST_CHECK_EQUAL(r3.max.y, 9.0f);
}

BOOST_AUTO_TEST_CASE(interval_make_intersection)
{
  using std::isnan;
  platform::floating_point_exceptions fpexceptions;

  interval<2, float> a(make_vector_from(1.0f, 2.0f),
                       make_vector_from(4.0f, 3.0f));
  interval<2, float> b(make_vector_from(2.0f, 1.0f),
                       make_vector_from(3.0f, 4.0f));
  interval<2, float> c(make_vector_from(6.0f, 7.0f),
                       make_vector_from(8.0f, 9.0f));
  auto r1 = make_interval_intersection(a);
  BOOST_CHECK_EQUAL(r1.min.x, 1.0f);
  BOOST_CHECK_EQUAL(r1.min.y, 2.0f);
  BOOST_CHECK_EQUAL(r1.max.x, 4.0f);
  BOOST_CHECK_EQUAL(r1.max.y, 3.0f);

  auto r2 = make_interval_intersection(a, b);
  BOOST_CHECK_EQUAL(r2.min.x, 2.0f);
  BOOST_CHECK_EQUAL(r2.min.y, 2.0f);
  BOOST_CHECK_EQUAL(r2.max.x, 3.0f);
  BOOST_CHECK_EQUAL(r2.max.y, 3.0f);

  auto r3 = make_interval_intersection(a, b, c);
  BOOST_CHECK(isnan(r3.min.x));
  BOOST_CHECK(isnan(r3.min.y));
  BOOST_CHECK(isnan(r3.max.x));
  BOOST_CHECK(isnan(r3.max.y));
}
