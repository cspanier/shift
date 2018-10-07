#ifndef SHIFT_MATH_QUATERNION_H
#define SHIFT_MATH_QUATERNION_H

#include <limits>
#include <cmath>
#include <shift/core/types.h>
#include "shift/math/utility.h"
#include "shift/math/vector.h"
#include "shift/math/matrix.h"

namespace shift::math
{
/// A template quaternion class of the form x * i + y * j + z * k + w.
template <typename T>
class quaternion
{
public:
  /// Default constructor initializing the quaternion with signaling_nan
  /// values.
  constexpr quaternion() noexcept = default;

  /// Copy constructor.
  constexpr quaternion(const quaternion& /*other*/) noexcept = default;

  /// Move constructor.
  constexpr quaternion(quaternion&& /*other*/) noexcept = default;

  /// Constructor.
  constexpr quaternion(T x, T y, T z, T w) noexcept;

  /// Constructor.
  quaternion(const T* data);

  /// Constructor that converts a rotation matrix to a quaternion.
  /// @remarks
  ///   This is only defined for 3x3 and 4x4 matrices. In the case of a 4x4
  ///   matrix, the upper left 3x3 sub-matrix is used to create the
  ///   quaternion.
  template <std::size_t N, ENABLE_IF(N == 3 || N == 4)>
  quaternion(const matrix<N, N, T>& matrix);

  /// Copy assignment operator.
  quaternion& operator=(const quaternion& /*other*/) noexcept = default;

  /// Move assignment operator.
  constexpr quaternion& operator=(quaternion&& /*other*/) noexcept = default;

  /// Quaternion addition.
  quaternion<T>& operator+=(const quaternion<T>& other);

  /// Quaternion multiplication.
  quaternion<T>& operator*=(const quaternion<T>& other);

  /// Quaternion negation.
  constexpr quaternion<T> operator-() const noexcept;

  /// @return
  ///    Returns a reference to a single component.
  constexpr T& operator()(std::size_t component) noexcept;

  /// @return
  ///    Returns a single component.
  constexpr T operator()(std::size_t component) const noexcept;

  /// @return
  ///    Returns the dot product of this and the other quaternion.
  T dot(const quaternion<T>& other) const;

  /// @return
  ///    Returns the normalized rotation axis.
  vector3<T> rotation_axis() const;

  /// @return
  ///    Returns the normalized rotation axis or (1, 0, 0) in the case
  ///    this quaternion is an identity quaternion.
  vector3<T> normalized_rotation_axis() const;

  /// Returns the address of the quaternion data.
  T* data();

  /// Returns the address of the vectquaternionor data.
  const T* data() const;

public:
  /// Calculates the spherical linear interpolation.
  static quaternion<T> slerp(const quaternion<T>& p, const quaternion<T>& q,
                             T t, bool shortestPath = false);

public:
  T x;
  T y;
  T z;
  T w;
};

/// Creates an identity quaternion.
template <typename T>
constexpr quaternion<T> make_identity_quaternion() noexcept
{
  return quaternion<T>(1, 0, 0, 0);
}

/// Creates a quaternion from an axis and and rotation angle around that axis.
template <typename T>
quaternion<T> make_quaternion_from_axis_angle(const vector3<T>& axis,
                                              T angle) noexcept
{
  using std::sin;
  using std::cos;
  T sin_angle = sin(angle / 2);
  T cos_angle = cos(angle / 2);
  return {sin_angle * axis(0), sin_angle * axis(1), sin_angle * axis(2),
          cos_angle};
}

template <typename T>
constexpr bool operator==(const quaternion<T>& lhs,
                          const quaternion<T>& rhs) noexcept
{
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

template <typename T>
constexpr bool operator!=(const quaternion<T>& lhs,
                          const quaternion<T>& rhs) noexcept
{
  return !(lhs == rhs);
}

/// Returns the norm of a quaternion, which is the squared magnitude.
/// @remarks
///   Note that a norm of type T is defined as a function that assigns a
///   strictly positive length or size to each argument of type T.
///   Implementations vary in chosing either the magnitude, or the squared
///   magnitude for this function. We use the latter one for performance
///   reasons. Please use abs() to get the magnitude of a quaternion.
template <typename T>
T norm(const quaternion<T>& any_quaternion) noexcept
{
  return any_quaternion.x * any_quaternion.x +
         any_quaternion.y * any_quaternion.y +
         any_quaternion.z * any_quaternion.z +
         any_quaternion.w * any_quaternion.w;
}

/// Returns the magnitude of a quaternion.
template <typename T>
T abs(const quaternion<T>& any_quaternion) noexcept
{
  using std::sqrt;
  return sqrt(norm(any_quaternion));
}

/// Returns the conjugate of a quaternion.
template <typename T>
constexpr quaternion<T> conjugate(const quaternion<T>& any_quaternion) noexcept
{
  return {-any_quaternion.x, -any_quaternion.y, -any_quaternion.z,
          any_quaternion.w};
}

/// Returns the inverse of a quaternion.
/// @remarks
///   Note that the inverse of a unit quaternion is equal to the conjugate of
///   that quaternion, which is less costly to compute.
template <typename T>
constexpr quaternion<T> inverse(const quaternion<T>& any_quaternion) noexcept
{
  auto squared_magnitude = norm(any_quaternion);
  if (squared_magnitude >= std::numeric_limits<T>::min())
  {
    return mul(quaternion<T>{-any_quaternion.x, -any_quaternion.y,
                             -any_quaternion.z, any_quaternion.w},
               1 / squared_magnitude);
  }
  else
    return quaternion<T>{T{0}, T{0}, T{0}, T{0}};
}

/// Normalizes a quaternion.
template <typename T>
quaternion<T> normalize(const quaternion<T>& any_quaternion) noexcept
{
  using std::abs;
  auto magnitude = abs(any_quaternion);
  if (magnitude < std::numeric_limits<T>::min())
    return {1, 0, 0, 0};
  auto inv_magnitude = 1 / magnitude;
  return {any_quaternion.x * inv_magnitude, any_quaternion.y * inv_magnitude,
          any_quaternion.z * inv_magnitude, any_quaternion.w * inv_magnitude};
}

/// Rotates a vector using a quaternion.
/// @remarks
///   There are several ways to rotate a vector. The mathematical approach
///   would be to use v' = q * v * inverse(q),
///   or v' = q * v * conjugate(q) for unit quaternions. However, it is faster
///   to convert the quaternion to a rotation matrix m and use v' = m * v.
template <typename T>
vector<3, T> rotate(const vector<3, T>& vector, const quaternion<T>& rotation)
{
  return make_rotation_matrix<3, 3>(rotation) * vector;
}

///
template <typename T>
vector<4, T> rotate(const vector<4, T>& vector, const quaternion<T>& rotation)
{
  auto direction =
    make_rotation_matrix<3, 3>(rotation) * swizzle<0, 1, 2>(vector);
  return make_vector_from(direction.x, direction.y, direction.z, vector.w);
}

template <typename T>
constexpr quaternion<T>::quaternion(T x, T y, T z, T w) noexcept
: x(x), y(y), z(z), w(w)
{
}

template <typename T>
quaternion<T>::quaternion(const T* data)
{
  x = *data;
  ++data;
  y = *data;
  ++data;
  z = *data;
  ++data;
  w = *data;
}

template <typename T>
template <std::size_t N, ENABLE_IF_DEF(N == 3 || N == 4)>
quaternion<T>::quaternion(const matrix<N, N, T>& matrix)
{
  T trace = matrix[0][0] + matrix[1][1] + matrix[2][2] + 1;
  if (trace > 0)
  {
    T trace_root = sqrtf(trace);
    T scale = 0.5f / trace_root;
    x = scale * (matrix[2][1] - matrix[1][2]);
    y = scale * (matrix[0][2] - matrix[2][0]);
    z = scale * (matrix[1][0] - matrix[0][1]);
    w = 0.5f * trace_root;
  }
  else
  {
    // Identify which major diagonal element has the greatest value.
    std::size_t column_select;
    if (matrix[0][0] > matrix[1][1])
      column_select = (matrix[0][0] > matrix[2][2]) ? 0 : 2;
    else
      column_select = (matrix[1][1] > matrix[2][2]) ? 1 : 2;

    if (column_select == 0)
    {
      T scale = sqrtf(matrix[0][0] - matrix[1][1] - matrix[2][2] + 1) * 2;

      x = (matrix[0][2] + matrix[2][0]) / scale;
      y = (matrix[1][2] + matrix[2][1]) / scale;
      z = 0.5f / scale;
      w = (matrix[0][1] + matrix[1][0]) / scale;
    }
    else if (column_select == 1)
    {
      T scale = sqrtf(matrix[1][1] - matrix[0][0] - matrix[2][2] + 1) * 2;

      x = (matrix[1][2] + matrix[2][1]) / scale;
      y = (matrix[0][2] + matrix[2][0]) / scale;
      z = (matrix[0][1] + matrix[1][0]) / scale;
      w = 0.5f / scale;
    }
    else
    {
      T scale = sqrtf(matrix[2][2] - matrix[0][0] - matrix[1][1] + 1) * 2;

      x = (matrix[1][2] + matrix[2][1]) / scale;
      y = (matrix[0][1] + matrix[1][0]) / scale;
      z = (matrix[0][2] + matrix[2][0]) / scale;
      w = 0.5f / scale;
    }
  }
}

template <typename T>
quaternion<T>& quaternion<T>::operator+=(const quaternion<T>& other)
{
  x += other.x;
  y += other.y;
  z += other.z;
  w += other.w;
  return *this;
}

template <typename T>
quaternion<T>& quaternion<T>::operator*=(const quaternion<T>& other)
{
  *this = *this * other;
  return *this;
}

template <typename T>
constexpr quaternion<T> quaternion<T>::operator-() const noexcept
{
  return {-x, -y, -z, -w};
}

template <typename T>
constexpr T& quaternion<T>::operator()(std::size_t component) noexcept
{
  BOOST_ASSERT(component < 4);
  switch (component)
  {
  case 0:
    return x;
  case 1:
    return y;
  case 2:
    return z;
  case 3:
    return w;
  }
  return x;
}

template <typename T>
constexpr T quaternion<T>::operator()(std::size_t component) const noexcept
{
  BOOST_ASSERT(component < 4);
  switch (component)
  {
  case 0:
    return x;
  case 1:
    return y;
  case 2:
    return z;
  case 3:
    return w;
  }
  return x;
}

template <typename T>
T quaternion<T>::dot(const quaternion<T>& other) const
{
  return w * other(0) + x * other(1) + y + other(2) + z + other(3);
}

template <typename T>
vector3<T> quaternion<T>::rotation_axis() const
{
  return math::vector3<float>(x, y, z);
}

template <typename T>
vector3<T> quaternion<T>::normalized_rotation_axis() const
{
  using std::abs;
  if (x == 0 && y == 0 && z == 0)
    return math::vector3<float>(1.0f, 0.0f, 0.0f);
  math::vector3<float> result(x, y, z);
  T length = abs(result);
  if (length < std::numeric_limits<T>::min())
    return math::vector3<float>(1.0f, 0.0f, 0.0f);
  return result * (1 / length);
}

template <typename T>
T* quaternion<T>::data()
{
  return &x;
}

template <typename T>
const T* quaternion<T>::data() const
{
  return &x;
}

template <typename T>
quaternion<T> slerp(const quaternion<T>& p, const quaternion<T>& q, T t,
                    bool shortestPath = false)
{
  using std::abs;
  T cosine = p.dot(q);
  quaternion<T> r;

  if (cosine < 0 && shortestPath)
  {
    cosine = -cosine;
    r = -q;
  }
  else
    r = q;

  if (!almost_equal(abs(cosine), 1))
  {
    T sine = sqrtf(1 - cosine * cosine);
    T angle = atan2f(sine, cosine);
    T invSine = 1 / sine;
    T coeffP = sinf((1 - t) * angle) * invSine;
    T coeffQ = sinf(t * angle) * invSine;
    return coeffP * p + coeffQ * r;
  }
  else
  {
    // This happens if p and q are either very close or almost inverse
    // of each other. While in the first case linear interpolation is
    // safe, we cannot actually solve the latter case as there are an
    // infinite number of possible interpolations.
    return ((1 - t) * p + t * r).normalize();
  }
}

///
template <typename T>
inline quaternion<T> operator+(const quaternion<T>& lhs,
                               const quaternion<T>& rhs)
{
  quaternion<T> result;
  for (std::size_t i = 0; i < 4; ++i)
    result[i] = lhs[i] + rhs[i];
  return result;
}

/// Hamilton product of two quaternions.
/// @remarks
///   Remember that quaternion multiplication is noncommutative (i.e.
///   generally q1 * q2 != q2 * q1).
/// @remarks
///   The product of two rotation quaternions q1 and q2 will be equivalent to
///   the rotation q1 followed by the rotation q2.
template <typename T>
constexpr quaternion<T> operator*(const quaternion<T>& lhs,
                                  const quaternion<T>& rhs) noexcept
{
  return {+lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y + lhs.w * rhs.x,
          -lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x + lhs.w * rhs.y,
          +lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w + lhs.w * rhs.z,
          -lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z + lhs.w * rhs.w};
}

///
template <typename T>
constexpr quaternion<T> operator*(const quaternion<T>& rotation,
                                  const vector<3, T>& vector) noexcept
{
  return rotation * quaternion<T>{vector.x, vector.y, vector.z, T{0}};
}

///
template <typename T>
constexpr quaternion<T> operator*(const vector<3, T>& vector,
                                  const quaternion<T>& rotation) noexcept
{
  return inverse(rotation) * vector;
}

///
template <typename T>
constexpr quaternion<T> operator*(const quaternion<T>& rotation,
                                  const vector<4, T>& vector) noexcept
{
  return rotation * quaternion<T>{vector.x / vector.w, vector.y / vector.w,
                                  vector.z / vector.w, T{0}};
}

///
template <typename T>
constexpr quaternion<T> operator*(const vector<4, T>& vector,
                                  const quaternion<T>& rotation) noexcept
{
  return inverse(rotation) * vector;
}

///
template <typename T>
constexpr quaternion<T> operator*(T lhs, const quaternion<T>& rhs) noexcept
{
  return {lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w};
}

///
template <typename T>
constexpr quaternion<T> operator*(const quaternion<T>& lhs, T rhs) noexcept
{
  return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs};
}

///
template <typename T>
inline std::ostream& operator<<(std::ostream& stream,
                                const quaternion<T>& any_quaternion)
{
  stream << "(";
  for (std::size_t i = 0; i < 4; ++i)
  {
    if (i != 0)
      stream << ",";
    stream << any_quaternion(i);
  }
  stream << ")";

  return stream;
}

///
template <typename T>
inline std::istream& operator>>(std::istream& stream,
                                quaternion<T>& any_quaternion)
{
  stream.get();
  for (std::size_t i = 0; i < 4; ++i)
  {
    if (i != 0)
      stream.get();
    if ((stream >> any_quaternion(i)).fail())
      break;
  }
  stream.get();

  BOOST_ASSERT(!stream.fail());

  return stream;
}
}

#endif
