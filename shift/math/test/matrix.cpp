#include <shift/math/matrix.h>
#include <shift/math/vector.h>
#include <shift/math/utility.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

using namespace shift::math;
using namespace shift::math::literals;

BOOST_AUTO_TEST_CASE(matrix_is_trivial)
{
  static_assert(std::is_trivial_v<matrix22<float>>);
  static_assert(std::is_trivial_v<matrix33<float>>);
  static_assert(std::is_trivial_v<matrix44<float>>);
  BOOST_CHECK(std::is_trivial_v<matrix22<float>>);
  BOOST_CHECK(std::is_trivial_v<matrix33<float>>);
  BOOST_CHECK(std::is_trivial_v<matrix44<float>>);
}

BOOST_AUTO_TEST_CASE(matrix_explicit_construction)
{
  auto m1 = make_matrix_from_column_major<2, 2, float>(1.0f, 2.0f, 3.0f, 4.0f);
  BOOST_CHECK_EQUAL(m1(0, 0), 1.0f);
  BOOST_CHECK_EQUAL(m1(1, 0), 2.0f);
  BOOST_CHECK_EQUAL(m1(0, 1), 3.0f);
  BOOST_CHECK_EQUAL(m1(1, 1), 4.0f);

  auto m2 = make_matrix_from_row_major<2, 2, float>(1.0f, 2.0f, 3.0f, 4.0f);
  BOOST_CHECK_EQUAL(m2(0, 0), 1.0f);
  BOOST_CHECK_EQUAL(m2(1, 0), 3.0f);
  BOOST_CHECK_EQUAL(m2(0, 1), 2.0f);
  BOOST_CHECK_EQUAL(m2(1, 1), 4.0f);
}

BOOST_AUTO_TEST_CASE(matrix_construction_from_column_vectors)
{
  auto v1 = make_vector_from(1.0f, 2.0f);
  auto v2 = make_vector_from(3.0f, 4.0f);
  auto m = make_matrix_from_column_major<2, 2, float>(v1, v2);
  BOOST_CHECK_EQUAL(m(0, 0), 1.0f);
  BOOST_CHECK_EQUAL(m(1, 0), 2.0f);
  BOOST_CHECK_EQUAL(m(0, 1), 3.0f);
  BOOST_CHECK_EQUAL(m(1, 1), 4.0f);
}

BOOST_AUTO_TEST_CASE(matrix_construction_from_float_c_array)
{
  float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};

  auto m1 = make_matrix_from_column_major<2, 2, float>(data);
  BOOST_CHECK_EQUAL(m1(0, 0), 1.0f);
  BOOST_CHECK_EQUAL(m1(1, 0), 2.0f);
  BOOST_CHECK_EQUAL(m1(0, 1), 3.0f);
  BOOST_CHECK_EQUAL(m1(1, 1), 4.0f);

  auto m2 = make_matrix_from_row_major<2, 2, float>(data);
  BOOST_CHECK_EQUAL(m2(0, 0), 1.0f);
  BOOST_CHECK_EQUAL(m2(1, 0), 3.0f);
  BOOST_CHECK_EQUAL(m2(0, 1), 2.0f);
  BOOST_CHECK_EQUAL(m2(1, 1), 4.0f);
}

BOOST_AUTO_TEST_CASE(matrix_construction_from_float_cpp_array)
{
  std::array<float, 4> data = {{1.0f, 2.0f, 3.0f, 4.0f}};

  auto m1 = make_matrix_from_column_major<2, 2, float>(data);
  BOOST_CHECK_EQUAL(m1(0, 0), 1.0f);
  BOOST_CHECK_EQUAL(m1(1, 0), 2.0f);
  BOOST_CHECK_EQUAL(m1(0, 1), 3.0f);
  BOOST_CHECK_EQUAL(m1(1, 1), 4.0f);

  auto m2 = make_matrix_from_row_major<2, 2, float>(data);
  BOOST_CHECK_EQUAL(m2(0, 0), 1.0f);
  BOOST_CHECK_EQUAL(m2(1, 0), 3.0f);
  BOOST_CHECK_EQUAL(m2(0, 1), 2.0f);
  BOOST_CHECK_EQUAL(m2(1, 1), 4.0f);
}

BOOST_AUTO_TEST_CASE(matrix_copy_construction)
{
  auto m1 = make_matrix_from_column_major<2, 2, float>(1.0f, 2.0f, 3.0f, 4.0f);
  matrix22<float> m2(m1);
  BOOST_CHECK_EQUAL(m2(0, 0), 1.0f);
  BOOST_CHECK_EQUAL(m2(1, 0), 2.0f);
  BOOST_CHECK_EQUAL(m2(0, 1), 3.0f);
  BOOST_CHECK_EQUAL(m2(1, 1), 4.0f);
}

BOOST_AUTO_TEST_CASE(matrix_assign)
{
  {
    matrix22<float> m1;
    m1.assign<detail::column_major, 0>(1.0f, 2.0f, 3.0f, 4.0f);
    BOOST_CHECK_EQUAL(m1(0, 0), 1.0f);
    BOOST_CHECK_EQUAL(m1(1, 0), 2.0f);
    BOOST_CHECK_EQUAL(m1(0, 1), 3.0f);
    BOOST_CHECK_EQUAL(m1(1, 1), 4.0f);
  }

  {
    matrix22<float> m2;
    m2.assign<detail::column_major, 0>(std::make_tuple(1.0f, 2.0f, 3.0f, 4.0f));
    BOOST_CHECK_EQUAL(m2(0, 0), 1.0f);
    BOOST_CHECK_EQUAL(m2(1, 0), 2.0f);
    BOOST_CHECK_EQUAL(m2(0, 1), 3.0f);
    BOOST_CHECK_EQUAL(m2(1, 1), 4.0f);
  }

  {
    matrix22<float> m2;
    m2.assign<detail::column_major, 0>(
      make_vector_from(1.0f, 2.0f, 3.0f, 4.0f));
    BOOST_CHECK_EQUAL(m2(0, 0), 1.0f);
    BOOST_CHECK_EQUAL(m2(1, 0), 2.0f);
    BOOST_CHECK_EQUAL(m2(0, 1), 3.0f);
    BOOST_CHECK_EQUAL(m2(1, 1), 4.0f);
  }
}

BOOST_AUTO_TEST_CASE(matrix_operators)
{
  BOOST_CHECK_EQUAL(
    (make_identity_matrix<3, 3, float>() * make_identity_matrix<3, 3, float>()),
    (make_identity_matrix<3, 3, float>()));
  BOOST_CHECK_EQUAL(
    (make_matrix_from_column_major<2, 3, float>(0.0f, 5.0f, 3.0f, 5.0f, 5.0f,
                                                2.0f) *
     make_matrix_from_column_major<3, 2, float>(3.0f, 3.0f, 4.0f, 4.0f, -2.0f,
                                                -2.0f)),
    (make_matrix_from_column_major<2, 2, float>(29.0f, 38.0f, -16.0f, 6.0f)));
}

BOOST_AUTO_TEST_CASE(matrix_sub_matrix)
{
  auto m = make_matrix_from_column_major<4, 4, int>(0, 1, 2, 3, 4, 5, 6, 7, 8,
                                                    9, 10, 11, 12, 13, 14, 15);
  BOOST_CHECK_EQUAL(
    sub_matrix(m, 0, 0),
    (make_matrix_from_column_major<3, 3, int>(5, 6, 7, 9, 10, 11, 13, 14, 15)));
  BOOST_CHECK_EQUAL(
    sub_matrix(m, 2, 1),
    (make_matrix_from_column_major<3, 3, int>(0, 1, 3, 8, 9, 11, 12, 13, 15)));
  BOOST_CHECK_EQUAL(
    sub_matrix(m, 3, 3),
    (make_matrix_from_column_major<3, 3, int>(0, 1, 2, 4, 5, 6, 8, 9, 10)));
}

BOOST_AUTO_TEST_CASE(matrix_determinant)
{
  BOOST_CHECK_EQUAL(determinant(make_matrix_from_column_major<2, 2, float>(
                      4.0f, 3.0f, 3.0f, 2.0f)),
                    -1);
  BOOST_CHECK_EQUAL(determinant(make_matrix_from_column_major<3, 3, float>(
                      4.0f, 4.0f, -2.0f, -1.0f, 5.0f, 0.0f, 1.0f, 3.0f, 0.0f)),
                    16);
}

BOOST_AUTO_TEST_CASE(matrix_inverse)
{
  {
    auto m = make_matrix_from_column_major<2, 2, float>(4.0f, 3.0f, 3.0f, 2.0f);
    BOOST_CHECK_EQUAL(inverse(m), (make_matrix_from_column_major<2, 2, float>(
                                    -2.0f, 3.0f, 3.0f, -4.0f)));
  }

  {
    auto m = make_matrix_from_column_major<2, 2, float>(4.0f, 3.0f, 3.0f, 2.0f);
    BOOST_CHECK_EQUAL(m * inverse(m), (make_identity_matrix<2, 2, float>()));
  }
  {
    auto m1 = make_rotation_matrix_x<3, 3>(1.0f);
    auto m2 = make_rotation_matrix_y<3, 3>(2.0f);
    auto m3 = make_rotation_matrix_z<3, 3>(3.0f);
    BOOST_CHECK_EQUAL(m1 * inverse(m1), (make_identity_matrix<3, 3, float>()));
    BOOST_CHECK_EQUAL(m2 * inverse(m2), (make_identity_matrix<3, 3, float>()));
    BOOST_CHECK_EQUAL(m3 * inverse(m3), (make_identity_matrix<3, 3, float>()));
  }
}

BOOST_AUTO_TEST_CASE(matrix_rotation)
{
  {
    const auto v = make_vector_from(1.0f, 2.0f);
    BOOST_CHECK_EQUAL((make_rotation_matrix_z<2, 2>(90_fdeg) * v),
                      make_vector_from(-2.0f, 1.0f));
  }
  {
    const auto v = make_vector_from(1.0f, 2.0f, 3.0f);
    BOOST_CHECK_EQUAL((make_rotation_matrix_x<3, 3>(90_fdeg) * v),
                      make_vector_from(1.0f, -3.0f, 2.0f));
    BOOST_CHECK_EQUAL((make_rotation_matrix_y<3, 3>(90_fdeg) * v),
                      make_vector_from(3.0f, 2.0f, -1.0f));
    BOOST_CHECK_EQUAL((make_rotation_matrix_z<3, 3>(90_fdeg) * v),
                      make_vector_from(-2.0f, 1.0f, 3.0f));
  }
  {
    const auto v = make_vector_from(1.0f, 2.0f, 3.0f, 1.0f);
    BOOST_CHECK_EQUAL((make_rotation_matrix_x<4, 4>(90_fdeg) * v),
                      make_vector_from(1.0f, -3.0f, 2.0f, 1.0f));
    BOOST_CHECK_EQUAL((make_rotation_matrix_y<4, 4>(90_fdeg) * v),
                      make_vector_from(3.0f, 2.0f, -1.0f, 1.0f));
    BOOST_CHECK_EQUAL((make_rotation_matrix_z<4, 4>(90_fdeg) * v),
                      make_vector_from(-2.0f, 1.0f, 3.0f, 1.0f));
  }
  /// ToDo: Test make_rotation_matrix(quaternion)
}

BOOST_AUTO_TEST_CASE(matrix_scale)
{
  /// ToDo: Test make_scale_matrix(scalar) and make_scale_matrix(vector).
}

BOOST_AUTO_TEST_CASE(matrix_transformation)
{
  {
    const auto v = make_vector_from(0.0f, 0.0f, 0.0f, 1.0f);
    auto rotation = make_rotation_matrix_x<4, 4>(90_fdeg);
    auto translation = make_translation_matrix<4, 4, float>(0.0f, 1.0f, 0.0f);
    BOOST_CHECK_EQUAL(rotation * translation * v,
                      make_vector_from(0.0f, 0.0f, 1.0f, 1.0f));
    BOOST_CHECK_EQUAL(translation * rotation * v,
                      make_vector_from(0.0f, 1.0f, 0.0f, 1.0f));
  }
  {
    auto v = make_vector_from(1.0f, 0.0f, 0.0f, 1.0f);
    auto m1 = make_rotation_matrix_x<4, 4>(1.0f);
    auto m2 = make_rotation_matrix_y<4, 4>(2.0f);
    auto m3 = make_rotation_matrix_z<4, 4>(3.0f);
    auto m1i = inverse(m1);
    auto m2i = inverse(m2);
    auto m3i = inverse(m3);
    BOOST_CHECK_EQUAL(m1 * m1i, (make_identity_matrix<4, 4, float>()));
    BOOST_CHECK_EQUAL(m2 * m2i, (make_identity_matrix<4, 4, float>()));
    BOOST_CHECK_EQUAL(m3 * m3i, (make_identity_matrix<4, 4, float>()));
    BOOST_CHECK_EQUAL((m2 * m1) * (m1i * m2i),
                      (make_identity_matrix<4, 4, float>()));
    BOOST_CHECK_EQUAL((m1i * m2i * m3i) * ((m3 * m2 * m1) * v), v);
  }
}

BOOST_AUTO_TEST_CASE(matrix_translation_rotation_translation)
{
  const auto bottom_left = make_vector_from(0.0f, 0.0f, 0.0f, 1.0f);
  const auto bottom_right = make_vector_from(1.0f, 0.0f, 0.0f, 1.0f);
  const auto top_right = make_vector_from(1.0f, 1.0f, 0.0f, 1.0f);
  const auto top_left = make_vector_from(0.0f, 1.0f, 0.0f, 1.0f);

  BOOST_CHECK_EQUAL(
    (make_translation_matrix<4, 4, float>(0.5f, 0.5f, 0.0f) *
     make_rotation_matrix_z<4, 4>(90_fdeg) *
     make_translation_matrix<4, 4, float>(-0.5f, -0.5f, 0.0f) * bottom_left),
    bottom_right);
  BOOST_CHECK_EQUAL(
    (make_translation_matrix<4, 4, float>(0.5f, 0.5f, 0.0f) *
     make_rotation_matrix_z<4, 4>(90_fdeg) *
     make_translation_matrix<4, 4, float>(-0.5f, -0.5f, 0.0f) * bottom_right),
    top_right);
  BOOST_CHECK_EQUAL(
    (make_translation_matrix<4, 4, float>(0.5f, 0.5f, 0.0f) *
     make_rotation_matrix_z<4, 4>(90_fdeg) *
     make_translation_matrix<4, 4, float>(-0.5f, -0.5f, 0.0f) * top_right),
    top_left);
  BOOST_CHECK_EQUAL(
    (make_translation_matrix<4, 4, float>(0.5f, 0.5f, 0.0f) *
     make_rotation_matrix_z<4, 4>(90_fdeg) *
     make_translation_matrix<4, 4, float>(-0.5f, -0.5f, 0.0f) * top_left),
    bottom_left);
}

BOOST_AUTO_TEST_CASE(matrix_orthographic_projection)
{
  auto m =
    make_orthographic_projection_matrix(-3.0f, 3.0f, -2.0f, 2.0f, 0.0f, 2.0f);
  BOOST_CHECK_EQUAL(m * make_vector_from(-2.7f, 0.0f, -0.2f, 1.0f),
                    make_vector_from(-0.9f, 0.0f, 0.1f, 1.0f));
  BOOST_CHECK_EQUAL(m * make_vector_from(2.7f, 0.0f, -1.8f, 1.0f),
                    make_vector_from(0.9f, 0.0f, 0.9f, 1.0f));
  BOOST_CHECK_EQUAL(m * make_vector_from(0.0f, -1.8f, -1.8f, 1.0f),
                    make_vector_from(0.0f, -0.9f, 0.9f, 1.0f));
  BOOST_CHECK_EQUAL(m * make_vector_from(0.0f, 1.8f, -0.2f, 1.0f),
                    make_vector_from(0.0f, 0.9f, 0.1f, 1.0f));
}

BOOST_AUTO_TEST_CASE(matrix_perspective_projection)
{
  auto m =
    make_perspective_projection_matrix<float>(90_fdeg, 2.0f, 1.0f, 11.0f);
  auto v1 = m * make_vector_from(-1.0f, 0.0f, -1.0f, 1.0f);
  auto v2 = m * make_vector_from(2.0f, 0.0f, -2.0f, 1.0f);
  auto v3 = m * make_vector_from(0.0f, -2.5f, -5.0f, 1.0f);
  auto v4 = m * make_vector_from(0.0f, 5.5f, -11.0f, 1.0f);

  v1 /= v1.w;
  v2 /= v2.w;
  v3 /= v3.w;
  v4 /= v4.w;

  BOOST_CHECK_EQUAL(v1, make_vector_from(-0.5f, 0.0f, 0.0f, 1.0f));
  BOOST_CHECK_EQUAL(v2, make_vector_from(0.5f, 0.0f, 0.55f, 1.0f));
  BOOST_CHECK_EQUAL(v3, make_vector_from(0.0f, -0.5f, 0.88f, 1.0f));
  BOOST_CHECK_EQUAL(v4, make_vector_from(0.0f, 0.5f, 1.0f, 1.0f));
}
