#include <shift/math/vector.h>
#include <shift/math/matrix.h>
#include <shift/core/algorithm.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

using namespace shift::math;

template <typename T>
using vector5 = vector<5, T>;

BOOST_AUTO_TEST_CASE(vector_is_trivial)
{
  static_assert(std::is_trivial_v<vector2<float>>);
  static_assert(std::is_trivial_v<vector3<float>>);
  static_assert(std::is_trivial_v<vector4<float>>);
  BOOST_CHECK(std::is_trivial_v<vector2<float>>);
  BOOST_CHECK(std::is_trivial_v<vector3<float>>);
  BOOST_CHECK(std::is_trivial_v<vector4<float>>);
}

BOOST_AUTO_TEST_CASE(vector_construction)
{
  vector2<float> v2f1{1.0f, 2.0f};
  vector2<float> v2f2(1.0f, 2.0f);
  vector2<float> v2f3 = {1.0f, 2.0f};
  vector2<float> v2f4{v2f1};
  vector2<float> v2f5(v2f2);
  vector2<float> v2f6 = v2f3;

  BOOST_CHECK_EQUAL(v2f1.x, 1.0f);
  BOOST_CHECK_EQUAL(v2f1.y, 2.0f);
  BOOST_CHECK_EQUAL(v2f2.x, 1.0f);
  BOOST_CHECK_EQUAL(v2f2.y, 2.0f);
  BOOST_CHECK_EQUAL(v2f3.x, 1.0f);
  BOOST_CHECK_EQUAL(v2f3.y, 2.0f);
  BOOST_CHECK_EQUAL(v2f4.x, 1.0f);
  BOOST_CHECK_EQUAL(v2f4.y, 2.0f);
  BOOST_CHECK_EQUAL(v2f5.x, 1.0f);
  BOOST_CHECK_EQUAL(v2f5.y, 2.0f);
  BOOST_CHECK_EQUAL(v2f6.x, 1.0f);
  BOOST_CHECK_EQUAL(v2f6.y, 2.0f);
}

BOOST_AUTO_TEST_CASE(vector_accessors)
{
  auto v2f = make_vector_from(1.0f, 2.0f);
  BOOST_CHECK_EQUAL(v2f(0), 1.0f);
  BOOST_CHECK_EQUAL(v2f(1), 2.0f);
  BOOST_CHECK_EQUAL(v2f.x, 1.0f);
  BOOST_CHECK_EQUAL(v2f.y, 2.0f);
  BOOST_CHECK_EQUAL(v2f.r, 1.0f);
  BOOST_CHECK_EQUAL(v2f.g, 2.0f);
  BOOST_CHECK_EQUAL(v2f.left, 1.0f);
  BOOST_CHECK_EQUAL(v2f.top, 2.0f);
  BOOST_CHECK_EQUAL(v2f.width, 1.0f);
  BOOST_CHECK_EQUAL(v2f.height, 2.0f);
  BOOST_CHECK_EQUAL(v2f.phi, 1.0f);
  BOOST_CHECK_EQUAL(v2f.theta, 2.0f);
  BOOST_CHECK_EQUAL(v2f.longitude, 1.0f);
  BOOST_CHECK_EQUAL(v2f.latitude, 2.0f);

  auto v3f = make_vector_from(1.0f, 2.0f, 3.0f);
  BOOST_CHECK_EQUAL(v3f(0), 1.0f);
  BOOST_CHECK_EQUAL(v3f(1), 2.0f);
  BOOST_CHECK_EQUAL(v3f(2), 3.0f);
  BOOST_CHECK_EQUAL(v3f.x, 1.0f);
  BOOST_CHECK_EQUAL(v3f.y, 2.0f);
  BOOST_CHECK_EQUAL(v3f.z, 3.0f);
  BOOST_CHECK_EQUAL(v3f.width, 1.0f);
  BOOST_CHECK_EQUAL(v3f.height, 2.0f);
  BOOST_CHECK_EQUAL(v3f.depth, 3.0f);
  BOOST_CHECK_EQUAL(v3f.phi, 1.0f);
  BOOST_CHECK_EQUAL(v3f.theta, 2.0f);
  BOOST_CHECK_EQUAL(v3f.psi, 3.0f);
  BOOST_CHECK_EQUAL(v3f.longitude, 1.0f);
  BOOST_CHECK_EQUAL(v3f.latitude, 2.0f);
  BOOST_CHECK_EQUAL(v3f.elevation, 3.0f);
  BOOST_CHECK_EQUAL(v3f.x, 1.0f);
  BOOST_CHECK_EQUAL(v3f.y, 2.0f);
  BOOST_CHECK_EQUAL(v3f.z, 3.0f);

  auto v4f = make_vector_from(1.0f, 2.0f, 3.0f, 4.0f);
  BOOST_CHECK_EQUAL(v4f(0), 1.0f);
  BOOST_CHECK_EQUAL(v4f(1), 2.0f);
  BOOST_CHECK_EQUAL(v4f(2), 3.0f);
  BOOST_CHECK_EQUAL(v4f(3), 4.0f);
  BOOST_CHECK_EQUAL(v4f.x, 1.0f);
  BOOST_CHECK_EQUAL(v4f.y, 2.0f);
  BOOST_CHECK_EQUAL(v4f.z, 3.0f);
  BOOST_CHECK_EQUAL(v4f.w, 4.0f);
  BOOST_CHECK_EQUAL(v4f.r, 1.0f);
  BOOST_CHECK_EQUAL(v4f.g, 2.0f);
  BOOST_CHECK_EQUAL(v4f.b, 3.0f);
  BOOST_CHECK_EQUAL(v4f.a, 4.0f);
  BOOST_CHECK_EQUAL(v4f.left, 1.0f);
  BOOST_CHECK_EQUAL(v4f.top, 2.0f);
  BOOST_CHECK_EQUAL(v4f.right, 3.0f);
  BOOST_CHECK_EQUAL(v4f.bottom, 4.0f);
  BOOST_CHECK_EQUAL(v4f.width, 3.0f);
  BOOST_CHECK_EQUAL(v4f.height, 4.0f);

  auto v5f = make_vector_from(1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
  BOOST_CHECK_EQUAL(v5f(0), 1.0f);
  BOOST_CHECK_EQUAL(v5f(1), 2.0f);
  BOOST_CHECK_EQUAL(v5f(2), 3.0f);
  BOOST_CHECK_EQUAL(v5f(3), 4.0f);
  BOOST_CHECK_EQUAL(v5f(4), 5.0f);
}

BOOST_AUTO_TEST_CASE(vector_construction_using_initializer_list)
{
  vector3<float> v = {1.0f, 2.0f, 3.0f};
  BOOST_CHECK_EQUAL(v.x, 1.0f);
  BOOST_CHECK_EQUAL(v.y, 2.0f);
  BOOST_CHECK_EQUAL(v.z, 3.0f);
}

BOOST_AUTO_TEST_CASE(vector_explicit_construction)
{
  vector4<float> v(1.0f, 2.0f, 3.0f, 4.0f);
  BOOST_CHECK_EQUAL(v.x, 1.0f);
  BOOST_CHECK_EQUAL(v.y, 2.0f);
  BOOST_CHECK_EQUAL(v.z, 3.0f);
  BOOST_CHECK_EQUAL(v.w, 4.0f);
}

BOOST_AUTO_TEST_CASE(vector_make_construction)
{
  auto v1 = make_vector_from(1.0f, 2.0f, 3.0f);
  BOOST_CHECK_EQUAL(v1.x, 1.0f);
  BOOST_CHECK_EQUAL(v1.y, 2.0f);
  BOOST_CHECK_EQUAL(v1.z, 3.0f);

  auto v2 = make_vector_from(make_vector_from(1.0f, 2.0f), 3.0f);
  BOOST_CHECK_EQUAL(v2.x, 1.0f);
  BOOST_CHECK_EQUAL(v2.y, 2.0f);
  BOOST_CHECK_EQUAL(v2.z, 3.0f);

  auto v3 = make_vector_from(1.0f, make_vector_from(2.0f, 3.0f));
  BOOST_CHECK_EQUAL(v3.x, 1.0f);
  BOOST_CHECK_EQUAL(v3.y, 2.0f);
  BOOST_CHECK_EQUAL(v3.z, 3.0f);
}

BOOST_AUTO_TEST_CASE(vector_copy_construction)
{
  auto v1 = make_vector_from(1.0f, 2.0f, 3.0f, 4.0f);
  vector4<float> v2(v1);
  BOOST_CHECK_EQUAL(v2.x, 1.0f);
  BOOST_CHECK_EQUAL(v2.y, 2.0f);
  BOOST_CHECK_EQUAL(v2.z, 3.0f);
  BOOST_CHECK_EQUAL(v2.w, 4.0f);
}

BOOST_AUTO_TEST_CASE(vector_carray_construction)
{
  const float values[4] = {1.0f, 2.0f, 3.0f, 4.0f};
  auto v1 = make_vector_from(values);
  BOOST_CHECK_EQUAL(v1.x, 1.0f);
  BOOST_CHECK_EQUAL(v1.y, 2.0f);
  BOOST_CHECK_EQUAL(v1.z, 3.0f);
  BOOST_CHECK_EQUAL(v1.w, 4.0f);
}

BOOST_AUTO_TEST_CASE(vector_array_construction)
{
  const std::array<float, 4> values{{1.0f, 2.0f, 3.0f, 4.0f}};
  auto v1 = make_vector_from(values);
  BOOST_CHECK_EQUAL(v1.x, 1.0f);
  BOOST_CHECK_EQUAL(v1.y, 2.0f);
  BOOST_CHECK_EQUAL(v1.z, 3.0f);
  BOOST_CHECK_EQUAL(v1.w, 4.0f);
}

BOOST_AUTO_TEST_CASE(vector_assignment_operators)
{
  {
    auto v1 = make_vector_from(4.0f, 5.0f, 6.0f);
    auto v2 = make_vector_from(1.0f, 2.0f, 3.0f);
    v1 += v2;
    BOOST_CHECK_EQUAL(v1.x, 5.0f);
    BOOST_CHECK_EQUAL(v1.y, 7.0f);
    BOOST_CHECK_EQUAL(v1.z, 9.0f);
  }
  {
    auto v1 = make_vector_from(4.0f, 5.0f, 6.0f);
    auto v2 = make_vector_from(1.0f, 2.0f, 3.0f);
    v1 -= v2;
    BOOST_CHECK_EQUAL(v1.x, 3.0f);
    BOOST_CHECK_EQUAL(v1.y, 3.0f);
    BOOST_CHECK_EQUAL(v1.z, 3.0f);
  }
  {
    auto v = make_vector_from(4.0f, 5.0f, 6.0f);
    v *= 2.0f;
    BOOST_CHECK_EQUAL(v.x, 8.0f);
    BOOST_CHECK_EQUAL(v.y, 10.0f);
    BOOST_CHECK_EQUAL(v.z, 12.0f);
  }
  {
    auto v = make_vector_from(4.0f, 5.0f, 6.0f);
    v /= 2.0f;
    BOOST_CHECK_EQUAL(v.x, 2.0f);
    BOOST_CHECK_EQUAL(v.y, 2.5f);
    BOOST_CHECK_EQUAL(v.z, 3.0f);
  }
}

BOOST_AUTO_TEST_CASE(vector_algorithm)
{
  using std::abs;
  using std::norm;
  {
    const auto v1 = make_vector_from(2.0f, 0.0f, 0.0f);
    const auto v2 = make_vector_from(2, 0, 0);
    const auto v3 = make_vector_from(2.0f, 0.0f, 0.0f);
    const auto v4 = make_vector_from(2, 0, 0);
    BOOST_CHECK_CLOSE(norm(v1), 4.0f, epsilon<float>);
    BOOST_CHECK_EQUAL(norm(v2), 4);
    BOOST_CHECK_CLOSE(abs(v3), 2.0f, epsilon<float>);
    BOOST_CHECK_EQUAL(abs(v4), 2);
  }
}

BOOST_AUTO_TEST_CASE(vector_std_min_max)
{
  const auto v1 = make_vector_from(1.0f, 0.0f, 0.0f);
  const auto v2 = make_vector_from(0.0f, 2.0f, 0.0f);
  BOOST_CHECK_EQUAL(std::min(v1, v2), v1);
  BOOST_CHECK_EQUAL(std::min(v2, v1), v1);
  BOOST_CHECK_EQUAL(std::max(v1, v2), v2);
  BOOST_CHECK_EQUAL(std::max(v2, v1), v2);
}

BOOST_AUTO_TEST_CASE(vector_math_min_max)
{
  const auto v1 = make_vector_from(1.0f, 5.0f, 3.0f);
  const auto v2 = make_vector_from(2.0f, 4.0f, 6.0f);
  BOOST_CHECK_EQUAL(min(v1), 1.0f);
  BOOST_CHECK_EQUAL(max(v1), 5.0f);
  BOOST_CHECK_EQUAL(min(v2), 2.0f);
  BOOST_CHECK_EQUAL(max(v2), 6.0f);
  BOOST_CHECK_EQUAL(min(v1, v2), make_vector_from(1.0f, 4.0f, 3.0f));
  BOOST_CHECK_EQUAL(min(v2, v1), make_vector_from(1.0f, 4.0f, 3.0f));
  BOOST_CHECK_EQUAL(max(v1, v2), make_vector_from(2.0f, 5.0f, 6.0f));
  BOOST_CHECK_EQUAL(max(v2, v1), make_vector_from(2.0f, 5.0f, 6.0f));
}

BOOST_AUTO_TEST_CASE(vector_add)
{
  const auto v1 = make_vector_from(1.0f, 2.0f, 3.0f);
  const auto v2 = make_vector_from(4.0f, 5.0f, 6.0f);
  BOOST_CHECK_EQUAL(v1 + v2,
                    make_vector_from(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z));
  BOOST_CHECK_EQUAL(v2 + v1,
                    make_vector_from(v2.x + v1.x, v2.y + v1.y, v2.z + v1.z));
  BOOST_CHECK_EQUAL(1 + v2, make_vector_from(1 + v2.x, 1 + v2.y, 1 + v2.z));
  BOOST_CHECK_EQUAL(v1 + 1, make_vector_from(v1.x + 1, v1.y + 1, v1.z + 1));
}

BOOST_AUTO_TEST_CASE(vector_sub)
{
  const auto v1 = make_vector_from(1.0f, 2.0f, 3.0f);
  const auto v2 = make_vector_from(4.0f, 5.0f, 6.0f);
  BOOST_CHECK_EQUAL(v1 - v2,
                    make_vector_from(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z));
  BOOST_CHECK_EQUAL(v2 - v1,
                    make_vector_from(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z));
  BOOST_CHECK_EQUAL(1 - v2, make_vector_from(1 - v2.x, 1 - v2.y, 1 - v2.z));
  BOOST_CHECK_EQUAL(v1 - 1, make_vector_from(v1.x - 1, v1.y - 1, v1.z - 1));
}

BOOST_AUTO_TEST_CASE(vector_mul)
{
  const auto v1 = make_vector_from(1.0f, 2.0f, 3.0f);
  const auto v2 = make_vector_from(4.0f, 5.0f, 6.0f);
  BOOST_CHECK_EQUAL(v1 * v2,
                    make_vector_from(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z));
  BOOST_CHECK_EQUAL(v2 * v1,
                    make_vector_from(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z));
  BOOST_CHECK_EQUAL(v1 * 2, make_vector_from(v1.x * 2, v1.y * 2, v1.z * 2));
  BOOST_CHECK_EQUAL(2 * v2, make_vector_from(2 * v2.x, 2 * v2.y, 2 * v2.z));
}

BOOST_AUTO_TEST_CASE(vector_div)
{
  const auto v1 = make_vector_from(1.0f, 2.0f, 3.0f);
  const auto v2 = make_vector_from(4.0f, 5.0f, 6.0f);
  BOOST_CHECK_EQUAL(v1 / v2,
                    make_vector_from(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z));
  BOOST_CHECK_EQUAL(v2 / v1,
                    make_vector_from(v2.x / v1.x, v2.y / v1.y, v2.z / v1.z));
  BOOST_CHECK_EQUAL(v1 / 2, make_vector_from(v1.x / 2, v1.y / 2, v1.z / 2));
  BOOST_CHECK_EQUAL(2 / v2, make_vector_from(2 / v2.x, 2 / v2.y, 2 / v2.z));
}

BOOST_AUTO_TEST_CASE(vector_dot)
{
  const auto v1 = make_vector_from(1.0f, 2.0f, 3.0f);
  const auto v2 = make_vector_from(4.0f, 5.0f, 6.0f);
  BOOST_CHECK_EQUAL(dot(v1, v2), v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

BOOST_AUTO_TEST_CASE(vector_cross)
{
  const auto x_axis = make_vector_from(1.0f, 0.0f, 0.0f);
  const auto y_axis = make_vector_from(0.0f, 1.0f, 0.0f);
  const auto z_axis = make_vector_from(0.0f, 0.0f, 1.0f);
  BOOST_CHECK_EQUAL(cross(x_axis, y_axis), z_axis);
  BOOST_CHECK_EQUAL(cross(y_axis, z_axis), x_axis);
  BOOST_CHECK_EQUAL(cross(z_axis, x_axis), y_axis);
}

BOOST_AUTO_TEST_CASE(vector_norm)
{
  using std::norm;
  const auto v1 = make_vector_from(1.0f, 2.0f, 2.0f);
  BOOST_CHECK_EQUAL(norm(v1), 9);
}

BOOST_AUTO_TEST_CASE(vector_abs)
{
  using std::abs;
  const auto v1 = make_vector_from(1.0f, 2.0f, 2.0f);
  BOOST_CHECK_EQUAL(abs(v1), 3);
}

BOOST_AUTO_TEST_CASE(vector_floor)
{
  const auto v1 = make_vector_from(-1.7f, -1.5f, -1.2f);
  const auto v2 = make_vector_from(-0.0f, 0.0f);
  const auto v3 = make_vector_from(1.2f, 1.5f, 1.7f);
  BOOST_CHECK_EQUAL(floor(v1), make_vector_from(-2.0f, -2.0f, -2.0f));
  BOOST_CHECK_EQUAL(floor(v2), make_vector_from(-0.0f, 0.0f));
  BOOST_CHECK_EQUAL(floor(v3), make_vector_from(1.0f, 1.0f, 1.0f));
}

BOOST_AUTO_TEST_CASE(vector_ceil)
{
  const auto v1 = make_vector_from(-1.7f, -1.5f, -1.2f);
  const auto v2 = make_vector_from(-0.0f, 0.0f);
  const auto v3 = make_vector_from(1.2f, 1.5f, 1.7f);
  BOOST_CHECK_EQUAL(ceil(v1), make_vector_from(-1.0f, -1.0f, -1.0f));
  BOOST_CHECK_EQUAL(ceil(v2), make_vector_from(-0.0f, 0.0f));
  BOOST_CHECK_EQUAL(ceil(v3), make_vector_from(2.0f, 2.0f, 2.0f));
}

BOOST_AUTO_TEST_CASE(vector_round)
{
  const auto v1 = make_vector_from(-1.7f, -1.5f, -1.2f);
  const auto v2 = make_vector_from(-0.0f, 0.0f);
  const auto v3 = make_vector_from(1.2f, 1.5f, 1.7f);
  BOOST_CHECK_EQUAL(round(v1), make_vector_from(-2.0f, -2.0f, -1.0f));
  BOOST_CHECK_EQUAL(round(v2), make_vector_from(-0.0f, 0.0f));
  BOOST_CHECK_EQUAL(round(v3), make_vector_from(1.0f, 2.0f, 2.0f));
}

BOOST_AUTO_TEST_CASE(vector_clamp)
{
  BOOST_CHECK_EQUAL(clamp(make_vector_from(1.5f, 1.5f, 1.5f),
                          make_vector_from(2.0f, 1.0f, 0.0f),
                          make_vector_from(3.0f, 2.0f, 1.0f)),
                    make_vector_from(2.0f, 1.5f, 1.0f));
}

BOOST_AUTO_TEST_CASE(vector_normalize)
{
  {
    const auto v1 = make_vector_from(2.0f, 0.0f, 0.0f);
    const auto v2 = make_vector_from(1.0f, 0.0f, 0.0f);
    const auto v3 = make_vector_from(0.1f, 0.0f, 0.0f);
    BOOST_CHECK_EQUAL(normalize(v1), v2);
    BOOST_CHECK_EQUAL(normalize(v2), v2);
    BOOST_CHECK_EQUAL(normalize(v3), v2);
  }
  {
    BOOST_CHECK_EQUAL(normalize(make_vector_from(1, 1, 1)),
                      normalize(make_vector_from(1.0f, 1.0f, 1.0f)));
  }
}

BOOST_AUTO_TEST_CASE(vector_reflect)
{
  using std::norm;
  {
    const auto direction = make_vector_from(0.0f, -1.0f);
    const auto normal = normalize(make_vector_from(1.0f, 2.0f));
    const auto expected_result = make_vector_from(0.8f, 0.6f);
    auto result = reflect(direction, normal);
    BOOST_CHECK_EQUAL(result, expected_result);
    BOOST_CHECK(norm(direction) == 1.0f);
    BOOST_CHECK(norm(expected_result) == 1.0f);
    BOOST_CHECK_EQUAL(reflect(-result, normal), -direction);
  }
  {
    const auto direction = make_vector_from(-1.0f, 0.0f);
    const auto normal = normalize(make_vector_from(1.0f, 2.0f));
    const auto expected_result = make_vector_from(-0.6f, 0.8f);
    auto result = reflect(direction, normal);
    BOOST_CHECK_EQUAL(result, expected_result);
    BOOST_CHECK(norm(direction) == 1.0f);
    BOOST_CHECK(norm(expected_result) == 1.0f);
    BOOST_CHECK_EQUAL(reflect(-result, normal), -direction);
  }
}

BOOST_AUTO_TEST_CASE(vector_step)
{
  BOOST_CHECK_EQUAL(step(0.5f, make_vector_from(0.0f, 0.5f, 2.0f)),
                    make_vector_from(0.0f, 1.0f, 1.0f));
  BOOST_CHECK_EQUAL(step(make_vector_from(0.5f, 0.5f, 0.5f),
                         make_vector_from(0.0f, 0.5f, 2.0f)),
                    make_vector_from(0.0f, 1.0f, 1.0f));
}

BOOST_AUTO_TEST_CASE(vector_mix)
{
  using shift::core::mix;
  BOOST_CHECK_EQUAL(mix(make_vector_from(0.0f, 1.0f, 2.0f),
                        make_vector_from(4.0f, 5.0f, 6.0f), 0.25f),
                    make_vector_from(1.0f, 2.0f, 3.0f));
  BOOST_CHECK_EQUAL(
    mix(make_vector_from(0.0f, 1.0f, 2.0f), make_vector_from(4.0f, 5.0f, 6.0f),
        make_vector_from(0.5f, 0.25f, 0.75f)),
    make_vector_from(2.0f, 2.0f, 5.0f));
}

/// A dummy type used for testing. The template argument has to be a
/// compile-time constant and be equal to 2 in order to hit the template
/// specialization.
template <int Const>
struct constexpr_test
{
  static constexpr bool value = false;
};

template <>
struct constexpr_test<2>
{
  static constexpr bool value = true;
};

BOOST_AUTO_TEST_CASE(vector_contexpr)
{
  constexpr vector3<int> v1(1, 2, 3);
  BOOST_STATIC_ASSERT((constexpr_test<v1(1)>::value));

  constexpr auto v2 = make_vector_from(1, 2, 3);
  BOOST_STATIC_ASSERT((constexpr_test<v2(1)>::value));
}
