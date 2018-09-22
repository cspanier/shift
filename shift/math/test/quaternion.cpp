#include <shift/math/quaternion.h>
#include <shift/math/utility.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

using namespace shift::math;

BOOST_AUTO_TEST_CASE(quaternion_is_trivial)
{
  static_assert(std::is_trivial_v<quaternion<float>>);
  BOOST_CHECK(std::is_trivial_v<quaternion<float>>);
}

BOOST_AUTO_TEST_CASE(quaternion_multiplication)
{
  const quaternion<float> q1{0.0f, 0.0f, 0.0f, 1.0f};
  const quaternion<float> qi{1.0f, 0.0f, 0.0f, 0.0f};
  const quaternion<float> qj{0.0f, 1.0f, 0.0f, 0.0f};
  const quaternion<float> qk{0.0f, 0.0f, 1.0f, 0.0f};
  BOOST_CHECK_EQUAL(q1 * q1, q1);
  BOOST_CHECK_EQUAL(q1 * qi, qi);
  BOOST_CHECK_EQUAL(q1 * qj, qj);
  BOOST_CHECK_EQUAL(q1 * qk, qk);
  BOOST_CHECK_EQUAL(qi * q1, qi);
  BOOST_CHECK_EQUAL(qi * qi, -q1);
  BOOST_CHECK_EQUAL(qi * qj, qk);
  BOOST_CHECK_EQUAL(qi * qk, -qj);
  BOOST_CHECK_EQUAL(qj * q1, qj);
  BOOST_CHECK_EQUAL(qj * qi, -qk);
  BOOST_CHECK_EQUAL(qj * qj, -q1);
  BOOST_CHECK_EQUAL(qj * qk, qi);
  BOOST_CHECK_EQUAL(qk * q1, qk);
  BOOST_CHECK_EQUAL(qk * qi, qj);
  BOOST_CHECK_EQUAL(qk * qj, -qi);
  BOOST_CHECK_EQUAL(qk * qk, -q1);
}

BOOST_AUTO_TEST_CASE(quaternion_conjugate)
{
  const quaternion<float> q1{1.0f, 2.0f, 3.0f, 4.0f};
  const quaternion<float> q2{2.0f, 3.0f, 2.0f, 1.0f};
  BOOST_CHECK_EQUAL(conjugate(q1 * q2), conjugate(q2) * conjugate(q1));
  // Check whether conjugate is an involutory function.
  BOOST_CHECK_EQUAL(conjugate(conjugate(q1)), q1);
}

BOOST_AUTO_TEST_CASE(quaternion_rotation_r3)
{
  using namespace shift::math::literals;

  auto x = make_vector_from(1.0f, 0.0f, 0.0f);
  auto y = make_vector_from(0.0f, 1.0f, 0.0f);
  auto z = make_vector_from(0.0f, 0.0f, 1.0f);
  auto r_x = make_quaternion_from_axis_angle(x, 90_fdeg);
  auto r_y = make_quaternion_from_axis_angle(y, 90_fdeg);
  auto r_z = make_quaternion_from_axis_angle(z, 90_fdeg);

  BOOST_CHECK(almost_equal(rotate(x, r_x), x));
  BOOST_CHECK(almost_equal(rotate(y, r_x), z));
  BOOST_CHECK(almost_equal(rotate(z, r_x), -y));
  BOOST_CHECK(almost_equal(rotate(x, r_y), -z));
  BOOST_CHECK(almost_equal(rotate(y, r_y), y));
  BOOST_CHECK(almost_equal(rotate(z, r_y), x));
  BOOST_CHECK(almost_equal(rotate(x, r_z), y));
  BOOST_CHECK(almost_equal(rotate(y, r_z), -x));
  BOOST_CHECK(almost_equal(rotate(z, r_z), z));
}

BOOST_AUTO_TEST_CASE(quaternion_rotation_r4)
{
  using namespace shift::math::literals;

  auto x = make_vector_from(1.0f, 0.0f, 0.0f);
  auto y = make_vector_from(0.0f, 1.0f, 0.0f);
  auto z = make_vector_from(0.0f, 0.0f, 1.0f);
  auto p_x = make_vector_from(+1.0f, 0.0f, 0.0f, 1.0f);
  auto p_y = make_vector_from(0.0f, +1.0f, 0.0f, 1.0f);
  auto p_z = make_vector_from(0.0f, 0.0f, +1.0f, 1.0f);
  auto n_x = make_vector_from(-1.0f, 0.0f, 0.0f, 1.0f);
  auto n_y = make_vector_from(0.0f, -1.0f, 0.0f, 1.0f);
  auto n_z = make_vector_from(0.0f, 0.0f, -1.0f, 1.0f);
  auto r_x = make_quaternion_from_axis_angle(x, 90_fdeg);
  auto r_y = make_quaternion_from_axis_angle(y, 90_fdeg);
  auto r_z = make_quaternion_from_axis_angle(z, 90_fdeg);

  BOOST_CHECK(almost_equal(rotate(p_x, r_x), p_x));
  BOOST_CHECK(almost_equal(rotate(p_y, r_x), p_z));
  BOOST_CHECK(almost_equal(rotate(p_z, r_x), n_y));
  BOOST_CHECK(almost_equal(rotate(p_x, r_y), n_z));
  BOOST_CHECK(almost_equal(rotate(p_y, r_y), p_y));
  BOOST_CHECK(almost_equal(rotate(p_z, r_y), p_x));
  BOOST_CHECK(almost_equal(rotate(p_x, r_z), p_y));
  BOOST_CHECK(almost_equal(rotate(p_y, r_z), n_x));
  BOOST_CHECK(almost_equal(rotate(p_z, r_z), p_z));
}
