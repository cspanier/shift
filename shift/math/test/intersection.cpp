#include <shift/math/intersection.h>
#include <shift/math/interval.h>
#include <shift/core/algorithm.h>
#include <shift/platform/fpexceptions.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

using namespace shift;
using namespace shift::math;

BOOST_AUTO_TEST_CASE(intersect_rectangle_rectangle)
{
  platform::floating_point_exceptions fpexceptions;

  auto p = [](auto x, auto y) { return make_vector_from(x, y); };
  auto r = [](auto p1, auto p2) { return make_interval_from(p1, p2); };

  // Same rectangles.
  BOOST_CHECK(
    intersection::rect_rect<true>(r(p(1, 1), p(4, 4)), r(p(1, 1), p(4, 4))));
  // First fully inside second.
  BOOST_CHECK(
    intersection::rect_rect<true>(r(p(2, 2), p(3, 3)), r(p(1, 1), p(4, 4))));
  // Second fully inside first.
  BOOST_CHECK(
    intersection::rect_rect<true>(r(p(1, 1), p(4, 4)), r(p(2, 2), p(3, 3))));
  // Second fully outside of first.
  BOOST_CHECK(
    !intersection::rect_rect<true>(r(p(1, 1), p(4, 4)), r(p(5, 1), p(8, 4))));

  // Only edge overlaps.
  BOOST_CHECK(
    intersection::rect_rect<true>(r(p(1, 1), p(3, 4)), r(p(3, 2), p(6, 5))));
  BOOST_CHECK(
    !intersection::rect_rect<false>(r(p(1, 1), p(3, 4)), r(p(3, 2), p(6, 5))));

  // Only top right corder overlaps.
  BOOST_CHECK(
    intersection::rect_rect<true>(r(p(1, 1), p(4, 4)), r(p(4, 4), p(8, 8))));
  BOOST_CHECK(
    !intersection::rect_rect<false>(r(p(1, 1), p(4, 4)), r(p(4, 4), p(8, 8))));

  // Only bottom left corder overlaps.
  BOOST_CHECK(
    intersection::rect_rect<true>(r(p(4, 4), p(8, 8)), r(p(1, 1), p(4, 4))));
  BOOST_CHECK(
    !intersection::rect_rect<false>(r(p(4, 4), p(8, 8)), r(p(1, 1), p(4, 4))));
}

BOOST_AUTO_TEST_CASE(intersect_point_triangle)
{
  auto v0 = make_vector_from(403.0f, 162.0f);
  auto v1 = make_vector_from(197.0f, 19.0f);
  auto v2 = make_vector_from(77.0f, 272.0f);

  // Check if the triangle corner points are within the triangle.
  BOOST_CHECK(intersection::point_triangle<true>(v0, v0, v1, v2));
  BOOST_CHECK(intersection::point_triangle<true>(v1, v0, v1, v2));
  BOOST_CHECK(intersection::point_triangle<true>(v2, v0, v1, v2));

  // Check three points that are known to be inside.
  auto i1 = make_vector_from(295.0f, 91.0f);
  auto i2 = make_vector_from(112.0f, 213.0f);
  auto i3 = make_vector_from(315.0f, 187.0f);
  BOOST_CHECK(intersection::point_triangle<true>(i1, v0, v1, v2));
  BOOST_CHECK(intersection::point_triangle<true>(i2, v0, v1, v2));
  BOOST_CHECK(intersection::point_triangle<true>(i3, v0, v1, v2));

  // Check three points that are known to be outside.
  auto o1 = make_vector_from(300.0f, 83.0f);
  auto o2 = make_vector_from(131.0f, 129.0f);
  auto o3 = make_vector_from(150.0f, 257.0f);
  BOOST_CHECK(!intersection::point_triangle<true>(o1, v0, v1, v2));
  BOOST_CHECK(!intersection::point_triangle<true>(o2, v0, v1, v2));
  BOOST_CHECK(!intersection::point_triangle<true>(o3, v0, v1, v2));
}
