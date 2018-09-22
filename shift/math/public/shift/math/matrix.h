#ifndef SHIFT_MATH_MATRIX_EX_H
#define SHIFT_MATH_MATRIX_EX_H

#include "shift/math/math.h"
#include "shift/math/matrix2x2.h"
#include "shift/math/matrix3x3.h"
#include "shift/math/matrix4x4.h"

namespace shift::math
{
/// Equality operator.
template <std::size_t Rows, std::size_t Columns, typename U, typename V>
constexpr bool operator==(const matrix<Rows, Columns, U>& lhs,
                          const matrix<Rows, Columns, V>& rhs) noexcept
{
  for (std::size_t column = 0u; column < Columns; ++column)
  {
    if (lhs.column_vector(column) != rhs.column_vector(column))
      return false;
  }
  return true;
}

/// Inequality operator.
/// @remarks
///   Implemented in terms of !(lhs == rhs).
template <std::size_t Rows, std::size_t Columns, typename U, typename V>
constexpr bool operator!=(const matrix<Rows, Columns, U>& lhs,
                          const matrix<Rows, Columns, V>& rhs) noexcept
{
  return !(lhs == rhs);
}

/// Per column vector comparison using almost_equal.
template <std::size_t Rows, std::size_t Columns, typename T>
std::enable_if_t<!std::is_integral_v<T>, bool> almost_equal(
  const matrix<Rows, Columns, T>& lhs, const matrix<Rows, Columns, T>& rhs,
  int units_in_the_last_place = 2)
{
  for (std::size_t column = 0u; column < Columns; ++column)
  {
    if (!almost_equal(lhs.column_vector(column), rhs.column_vector(column),
                      units_in_the_last_place))
    {
      return false;
    }
  }
  return true;
}

/// Matrix-matrix multiplication
template <std::size_t Columns1Rows2, std::size_t Rows1, std::size_t Columns2,
          typename T, typename U, ENABLE_IF(Columns2 > 1)>
constexpr auto operator*(const matrix<Rows1, Columns1Rows2, T>& lhs,
                         const matrix<Columns1Rows2, Columns2, U>& rhs) noexcept
{
  matrix<Rows1, Columns2, decltype(std::declval<T>() * std::declval<U>())>
    result{};
  for (std::size_t column = 0; column < Columns2; ++column)
  {
    for (std::size_t row = 0; row < Rows1; ++row)
      result(row, column) = dot(lhs.row_vector(row), rhs.column_vector(column));
  }
  return result;
}

/// Element-wise matrix-matrix division.
template <std::size_t Rows, std::size_t Columns, typename T, typename U>
constexpr auto operator/(const matrix<Rows, Columns, T>& lhs,
                         const matrix<Rows, Columns, U>& rhs)
{
  matrix<Rows, Columns, decltype(std::declval<T>() / std::declval<U>())>
    result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = lhs(row, column) / rhs(row, column);
  }
  return result;
}

/// Element-wise matrix-scalar division.
template <std::size_t Rows, std::size_t Columns, typename T, typename U,
          std::enable_if_t<!is_matrix_v<U>>* = nullptr>
constexpr auto operator/(const matrix<Rows, Columns, T>& lhs, U rhs)
{
  matrix<Rows, Columns, decltype(std::declval<T>() / std::declval<U>())>
    result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = lhs(row, column) / rhs;
  }
  return result;
}

/// String serialization.
template <std::size_t Rows, std::size_t Columns, typename T>
std::ostream& operator<<(std::ostream& stream,
                         const matrix<Rows, Columns, T>& matrix)
{
  stream << "(";
  for (std::size_t column = 0; column < Columns; ++column)
  {
    if (column != 0)
      stream << ", ";
    stream << matrix.column_vector(column);
  }
  stream << ")";

  return stream;
}

/// Returns a (Columns-1)x(Columns-1) sub-matrix of the argument by cutting
/// one row and one column.
/// @remarks
///   This method is only defined for matrices of 3x3 dimension or larger.
template <std::size_t Rows, std::size_t Columns, typename T,
          ENABLE_IF((Columns >= 3) && (Rows >= 3))>
constexpr auto sub_matrix(const matrix<Rows, Columns, T>& any_matrix,
                          std::size_t cut_row, std::size_t cut_column)
{
  static_assert(
    (Columns >= 3) && (Rows >= 3),
    "sub_matrix is defined only for matrices with more than two dimensions.");
  matrix<Rows - 1, Columns - 1, T> result{};
  for (std::size_t column = 0; column < Columns - 1; ++column)
  {
    for (std::size_t row = 0; row < Rows - 1; ++row)
    {
      result(row, column) =
        any_matrix(row < cut_row ? row : row + 1,
                   column < cut_column ? column : column + 1);
    }
  }
  return result;
}

/// Returns the determinant of a NxN matrix.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr T determinant(const matrix<Rows, Columns, T>& any_matrix) noexcept
{
  auto result = T{0};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    result += (static_cast<int>(column % 2) * -2 + 1) * any_matrix(0, column) *
              determinant(sub_matrix(any_matrix, 0, column));
  }
  return result;
}

/// Returns the transposed matrix of the argument.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr matrix<Columns, Rows, T> transpose(
  const matrix<Rows, Columns, T>& any_matrix) noexcept
{
  matrix<Columns, Rows, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = any_matrix(column, row);
  }
  return result;
}

/// Returns the inverse of an NxN matrix.
template <std::size_t Rows, std::size_t Columns, typename T,
          ENABLE_IF(Rows > 2 && Columns > 2)>
constexpr auto inverse(const matrix<Rows, Columns, T>& any_matrix) noexcept
{
  matrix<Columns, Rows, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
    {
      result(row, column) = (static_cast<int>((row + column + 1) % 2) * 2 - 1) *
                            determinant(sub_matrix(any_matrix, column, row));
    }
  }
  return result / determinant(any_matrix);
}

/// Returns an identity matrix.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_identity_matrix() noexcept
{
  matrix<Rows, Columns, T> result{};
  for (std::size_t i = 0; i < std::min(Rows, Columns); ++i)
    result(i, i) = T{1};
  return result;
}

/// Construct a matrix from a one dimensional C-style array of values stored
/// in column major order.
/// @remarks
///   If you need to initialize the matrix from a pointer to a memory region,
///   you may use make_matrix_from_column_major(*reinterpret_cast<
///     const T(*)[Rows * Columns]>(pointer)).
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_matrix_from_column_major(
  const T (&array)[Rows * Columns]) noexcept
{
  return matrix<Rows, Columns, T>(column_major{}, array);
}

/// Construct a matrix from a one dimensional C-style array of values
/// stored in row major order.
/// @remarks
///   If you need to initialize the matrix from a pointer to a memory
///   region, you may use make_matrix_from_row_major(*reinterpret_cast<
///     const T(*)[Rows * Columns]>(pointer)).
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_matrix_from_row_major(
  const T (&array)[Rows * Columns]) noexcept
{
  return matrix<Rows, Columns, T>(row_major{}, array);
}

/// Construct a matrix from a std::array of values stored in column major
/// order.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_matrix_from_column_major(
  const std::array<T, Rows * Columns>& array) noexcept
{
  return matrix<Rows, Columns, T>(column_major{}, array);
}

/// Construct a matrix from a std::array of values stored in row major order.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_matrix_from_row_major(
  const std::array<T, Rows * Columns>& array) noexcept
{
  return matrix<Rows, Columns, T>(row_major{}, array);
}

///
template <std::size_t Rows, std::size_t Columns, typename T, typename... Ts,
          std::enable_if_t<(sizeof...(Ts) > 1)>* = nullptr>
constexpr auto make_matrix_from_column_major(Ts&&... values) noexcept
{
  return matrix<Rows, Columns, T>(column_major{}, std::forward<Ts>(values)...);
}

///
template <std::size_t Rows, std::size_t Columns, typename T, typename... Ts,
          std::enable_if_t<(sizeof...(Ts) > 1)>* = nullptr>
constexpr auto make_matrix_from_row_major(Ts&&... values) noexcept
{
  return matrix<Rows, Columns, T>(row_major{}, std::forward<Ts>(values)...);
}

/// Returns a translation matrix.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_translation_matrix(const vector<Rows - 1, T>& v) noexcept
{
  if constexpr (Rows == 3 && Columns == 3)
  {
    // clang-format off
    return matrix<3, 3, T>(column_major{},
                           1.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f,
                           v(0), v(1), 1.0f);
    // clang-format on
  }
  else if constexpr (Rows == 4 && Columns == 4)
  {
    // clang-format off
    return matrix<4, 4, T>(column_major{},
                           1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           v(0), v(1), v(2), 1.0f);
    // clang-format on
  }
  else
  {
    // Not implemented.
    return matrix<Rows, Columns, T>{};
  }
}

/// Returns a translation matrix.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_translation_matrix(const vector<Rows, T>& v) noexcept
{
  if constexpr (Rows == 3 && Columns == 3)
  {
    // clang-format off
    return matrix<3, 3, T>(column_major{},
                           1.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f,
                           v(0), v(1), v(2));
    // clang-format on
  }
  else if constexpr (Rows == 4 && Columns == 4)
  {
    // clang-format off
    return matrix<4, 4, T>(column_major{},
                           1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           v(0), v(1), v(2), v(3));
    // clang-format on
  }
  else
  {
    // Not implemented.
    return matrix<Rows, Columns, T>{};
  }
}

/// Returns a translation matrix.
template <std::size_t Rows, std::size_t Columns, typename T, typename... Ts>
constexpr auto make_translation_matrix(Ts&&... axis) noexcept
{
  if constexpr (Rows == 3 && Columns == 3 && sizeof...(Ts) == 2)
  {
    // clang-format off
    return matrix<3, 3, T>(column_major{},
                           1.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f,
                           T{axis}..., 1.0f);
    // clang-format on
  }
  else if constexpr (Rows == 3 && Columns == 3 && sizeof...(Ts) == 3)
  {
    // clang-format off
    return matrix<3, 3, T>(column_major{},
                           1.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f,
                           T{axis}...);
    // clang-format on
  }
  else if constexpr (Rows == 4 && Columns == 4 && sizeof...(Ts) == 3)
  {
    // clang-format off
    return matrix<4, 4, T>(column_major{},
                           1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           T{axis}..., 1.0f);
    // clang-format on
  }
  else if constexpr (Rows == 4 && Columns == 4 && sizeof...(Ts) == 4)
  {
    // clang-format off
    return matrix<4, 4, T>(column_major{},
                           1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           T{axis}...);
    // clang-format on
  }
  else
  {
    // Not implemented.
    return matrix<Rows, Columns, T>{};
  }
}

/// Returns a rotation matrix around the positive X axis.
/// @remarks
///   The resulting matrix will have the following form:
///   \f[\left[ \begin{matrix}
///      1 & 0 & 0 & \cdots  & 0  \cr
///      0 & \cos(\alpha) & -\sin(\alpha) & \cdots  & 0  \cr
///      0 & \sin(\alpha) & \cos(\alpha) & \cdots  & 0  \cr
///      \vdots & \vdots & \vdots & \ddots  & \vdots   \cr
///      0 & 0 & 0 & \cdots & 1
///   \end{matrix} \right]\f]
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_rotation_matrix_x(T angle)
{
  static_assert(Rows >= 3 && Columns >= 3);

  auto s = static_cast<T>(sin(angle));
  auto c = static_cast<T>(cos(angle));
  if constexpr (Rows == 3 && Columns == 3)
  {
    // clang-format off
    return make_matrix_from_column_major<3, 3, T>(
      T{1}, T{0}, T{0},
      T{0},    c,    s,
      T{0},   -s,    c);
    // clang-format on
  }
  else if constexpr (Rows == 4 && Columns == 4)
  {
    // clang-format off
    return make_matrix_from_column_major<4, 4, T>(
      T{1}, T{0}, T{0}, T{0},
      T{0},    c,    s, T{0},
      T{0},   -s,    c, T{0},
      T{0}, T{0}, T{0}, T{1});
    // clang-format on
  }
  else
  {
    auto result = make_identity_matrix<Rows, Columns, T>();
    // clang-format off
    result(1, 1) = c; result(1, 2) = -s;
    result(2, 1) = s; result(2, 2) = c;
    // clang-format on
    return result;
  }
}

/// Returns a rotation matrix around the positive Y axis.
/// @remarks
///   The resulting matrix will have the following form:
///   \f[\left[ \begin{matrix}
///      \cos(\alpha) & 0 & \sin(\alpha) & \cdots  & 0  \cr
///      0 & 1 & 0 & \cdots  & 0  \cr
///      -\sin(\alpha) & 0 & \cos(\alpha) & \cdots  & 0  \cr
///      \vdots & \vdots & \vdots & \ddots  & \vdots   \cr
///      0 & 0 & 0 & \cdots & 1
///   \end{matrix} \right]\f]
template <std::size_t Rows, std::size_t Columns, typename T>
auto make_rotation_matrix_y(T angle)
{
  static_assert(Rows >= 3 && Columns >= 3);

  auto s = static_cast<T>(sin(angle));
  auto c = static_cast<T>(cos(angle));
  if constexpr (Rows == 3 && Columns == 3)
  {
    // clang-format off
    return make_matrix_from_column_major<3, 3, T>(
         c, T{0},   -s,
      T{0}, T{1}, T{0},
         s, T{0},    c);
    // clang-format on
  }
  else if constexpr (Rows == 4 && Columns == 4)
  {
    // clang-format off
    return make_matrix_from_column_major<4, 4, T>(
         c, T{0},   -s, T{0},
      T{0}, T{1}, T{0}, T{0},
         s, T{0},    c, T{0},
      T{0}, T{0}, T{0}, T{1});
    // clang-format on
  }
  else
  {
    auto result = make_identity_matrix<Rows, Columns, T>();
    // clang-format off
    result(0, 0) = c; result(0, 2) = s;
    result(2, 0) = -s; result(2, 2) = c;
    // clang-format on
    return result;
  }
}

/// Returns a rotation matrix around the positive Z axis.
/// @remarks
///   The resulting matrix will have the following form:
///   \f[\left[ \begin{matrix}
///      \cos(\alpha) & -\sin(\alpha) & 0 & \cdots  & 0  \cr
///      \sin(\alpha) & \cos(\alpha) & 0 & \cdots  & 0  \cr
///      0 & 0 & 1 & \cdots  & 0  \cr
///      \vdots & \vdots & \vdots & \ddots  & \vdots   \cr
///      0 & 0 & 0 & \cdots & 1
///   \end{matrix} \right]\f]
template <std::size_t Rows, std::size_t Columns, typename T>
auto make_rotation_matrix_z(T angle)
{
  static_assert(Rows >= 2 && Columns >= 2);

  auto s = static_cast<T>(sin(angle));
  auto c = static_cast<T>(cos(angle));
  if constexpr (Rows == 2 && Columns == 2)
  {
    // clang-format off
    return make_matrix_from_column_major<2, 2, T>(
       c, s,
      -s, c);
    // clang-format on
  }
  else if constexpr (Rows == 3 && Columns == 3)
  {
    // clang-format off
    return make_matrix_from_column_major<3, 3, T>(
         c,    s, T{0},
        -s,    c, T{0},
      T{0}, T{0}, T{1});
    // clang-format on
  }
  else if constexpr (Rows == 4 && Columns == 4)
  {
    // clang-format off
    return make_matrix_from_column_major<4, 4, T>(
         c,    s, T{0}, T{0},
        -s,    c, T{0}, T{0},
      T{0}, T{0}, T{1}, T{0},
      T{0}, T{0}, T{0}, T{1});
    // clang-format on
  }
  else
  {
    auto result = make_identity_matrix<Rows, Columns, T>();
    // clang-format off
    result(0, 0) = c; result(0, 1) = -s;
    result(1, 0) = s; result(1, 1) = c;
    // clang-format on
    return result;
  }
}

/// Builds a \f$Rows\times Columns\f$ rotation matrix from a quaternion.
template <std::size_t Rows, std::size_t Columns, typename T>
auto make_rotation_matrix(const quaternion<T>& rotation)
{
  static_assert(Rows >= 3 && Columns >= 3);

  T xx = rotation.x * rotation.x;
  T xy = rotation.x * rotation.y;
  T xz = rotation.x * rotation.z;
  T xw = rotation.x * rotation.w;

  T yy = rotation.y * rotation.y;
  T yz = rotation.y * rotation.z;
  T yw = rotation.y * rotation.w;

  T zz = rotation.z * rotation.z;
  T zw = rotation.z * rotation.w;

  if constexpr (Rows == 3 && Columns == 3)
  {
    // clang-format off
    return make_matrix_from_column_major<3, 3, T>(
      1 - 2 * (yy + zz),     2 * (xy + zw),     2 * (xz - yw),
          2 * (xy - zw), 1 - 2 * (xx + zz),     2 * (yz + xw),
          2 * (xz + yw),     2 * (yz - xw), 1 - 2 * (xx + yy));
    // clang-format on
  }
  else if constexpr (Rows == 4 && Columns == 4)
  {
    // clang-format off
    return make_matrix_from_column_major<4, 4, T>(
      1 - 2 * (yy + zz),     2 * (xy + zw),     2 * (xz - yw), T{0},
          2 * (xy - zw), 1 - 2 * (xx + zz),     2 * (yz + xw), T{0},
          2 * (xz + yw),     2 * (yz - xw), 1 - 2 * (xx + yy), T{0},
                   T{0},              T{0},              T{0}, T{1});
    // clang-format on
  }
  else
  {
    auto result = make_identity_matrix<Rows, Columns, T>();
    // clang-format off
    result(0, 0) = 1 - 2 * (yy + zz);
    result(1, 0) =     2 * (xy + zw);
    result(2, 0) =     2 * (xz - yw);

    result(0, 1) =     2 * (xy - zw);
    result(1, 1) = 1 - 2 * (xx + zz);
    result(2, 1) =     2 * (yz + xw);

    result(0, 2) =     2 * (xz + yw);
    result(1, 2) =     2 * (yz - xw);
    result(2, 2) = 1 - 2 * (xx + yy);
    // clang-format on
    return result;
  }
}

/// Returns a scale matrix.
/// @remarks
///   The resulting matrix will have the following form:
///   \f[\left[ \begin{matrix}
///      factor & 0 & 0 & \cdots  & 0  \cr
///      0 & factor & 0 & \cdots  & 0  \cr
///      0 & 0 & factor & \cdots  & 0  \cr
///      \vdots & \vdots & \vdots & \ddots  & \vdots   \cr
///      0 & 0 & 0 & \cdots & 1
///   \end{matrix} \right]\f]
template <std::size_t Rows, std::size_t Columns, std::size_t Dimensions,
          typename T>
constexpr auto make_scale_matrix(T factor)
{
  static_assert(Dimensions >= Rows && Dimensions >= Columns);

  if constexpr (Rows == 3 && Columns == 3 && Dimensions == 3)
  {
    // clang-format off
    return make_matrix_from_column_major<3, 3, T>(
      factor,   T{0},   T{0},
        T{0}, factor,   T{0},
        T{0},   T{0}, factor);
    // clang-format on
  }
  else if constexpr (Rows == 4 && Columns == 4 && Dimensions == 3)
  {
    // clang-format off
    return make_matrix_from_column_major<4, 4, T>(
      factor,   T{0},   T{0}, T{0},
        T{0}, factor,   T{0}, T{0},
        T{0},   T{0}, factor, T{0},
        T{0},   T{0},   T{0}, T{1});
    // clang-format on
  }
  else
  {
    matrix<Rows, Columns, T> result{};
    for (std::size_t column = 0; column < Columns; ++column)
    {
      for (std::size_t row = 0; row < Rows; ++row)
        result(row, column) = T{0};
    }
    for (std::size_t i = 0; i < Rows && i < Columns && i < Dimensions; ++i)
      result(i, i) = factor;
    for (std::size_t i = Dimensions; i < Rows && i < Columns; ++i)
      result(i, i) = T{1};
    return result;
  }
}

/// Returns a scale matrix.
/// @remarks
///   The resulting matrix will have the following form:
///   \f[\left[ \begin{matrix}
///      factor_0 & 0 & 0 & \cdots  & 0  \cr
///      0 & factor_1 & 0 & \cdots  & 0  \cr
///      0 & 0 & factor_2 & \cdots  & 0  \cr
///      \vdots & \vdots & \vdots & \ddots  & \vdots   \cr
///      0 & 0 & 0 & \cdots & 1
///   \end{matrix} \right]\f]
template <std::size_t Rows, std::size_t Columns, std::size_t Dimensions,
          typename T>
constexpr auto make_scale_matrix(const vector<Dimensions, T>& factors)
{
  static_assert(Dimensions <= Rows && Dimensions <= Columns);

  if constexpr (Rows == 3 && Columns == 3 && Dimensions == 3)
  {
    // clang-format off
    return make_matrix_from_column_major<3, 3, T>(
      factors(0),       T{0},       T{0},
            T{0}, factors(1),       T{0},
            T{0},       T{0}, factors(2));
    // clang-format on
  }
  else if constexpr (Rows == 4 && Columns == 4 && Dimensions == 3)
  {
    // clang-format off
    return make_matrix_from_column_major<4, 4, T>(
      factors(0),       T{0},       T{0}, T{0},
            T{0}, factors(1),       T{0}, T{0},
            T{0},       T{0}, factors(2), T{0},
            T{0},       T{0},       T{0}, T{1});
    // clang-format on
  }
  else
  {
    matrix<Rows, Columns, T> result{};
    for (std::size_t column = 0; column < Columns; ++column)
    {
      for (std::size_t row = 0; row < Rows; ++row)
        result(row, column) = T{0};
    }
    for (std::size_t i = 0; i < std::min(Rows, Columns) && i < Dimensions; ++i)
      result(i, i) = factors(i);
    for (std::size_t i = Dimensions; i < std::min(Rows, Columns); ++i)
      result(i, i) = T{1};
    return result;
  }
}

/// Returns a view matrix.
template <typename T>
matrix<4, 4, T> make_look_at_matrix(const vector3<T>& eye,
                                    const vector3<T>& target,
                                    const vector3<T>& upvec) noexcept
{
  const auto zaxis = normalize(eye - target);
  const auto xaxis = normalize(cross(zaxis, upvec));
  const auto yaxis = cross(xaxis, zaxis);

  // clang-format off
  return make_matrix_from_row_major<4, 4, T>(
      xaxis(0), xaxis(1), xaxis(2), -dot(xaxis, eye),
      yaxis(0), yaxis(1), yaxis(2), -dot(yaxis, eye),
      zaxis(0), zaxis(1), zaxis(2), -dot(zaxis, eye),
          T{0},     T{0},     T{0},             T{1});
  // clang-format on
}

/// Returns an inverse view matrix.
template <typename T>
matrix<4, 4, T> make_inv_look_at_matrix(const vector3<T>& eye,
                                        const vector3<T>& target,
                                        const vector3<T>& upvec) noexcept
{
  const auto zaxis = normalize(target - eye);
  const auto xaxis = normalize(cross(upvec, zaxis));
  const auto yaxis = cross(zaxis, xaxis);

  // clang-format off
  return make_matrix_from_row_major<4, 4, T>(
      xaxis(0), yaxis(0), zaxis(0), -eye(0),
      xaxis(1), yaxis(1), zaxis(1), -eye(1),
      xaxis(2), yaxis(2), zaxis(2), -eye(2),
          T{0},     T{0},     T{0},    T{1});
  // clang-format on
}

/// Returns an infinite perspective 4x4 projection matrix, which is used to
/// transform points from view space into clip space.
/// @remarks
///   Note that clip space is supposed to range from zero to one in z direction,
///   and not from negative one to one (which is default for OpenGL; see
///   glClipControl() for details).
template <typename T>
matrix<4, 4, T> make_perspective_projection_matrix(T fov_y, T aspect_ratio,
                                                   T near_plane) noexcept
{
  const T h = 1 / std::tan(fov_y / 2);
  const T w = h / aspect_ratio;
  const T d = -2 * near_plane;

  // clang-format off
  return make_matrix_from_row_major<4, 4, T>(
       w, T{0},  T{0}, T{0},
    T{0},    h,  T{0}, T{0},
    T{0}, T{0}, T{-1},    d,
    T{0}, T{0}, T{-1}, T{0});
  // clang-format on
}

/// Returns a finite perspective 4x4 projection matrix, which is used to
/// transform points from view space into clip space.
/// @remarks
///   Note that clip space is supposed to range from zero to one in z direction,
///   and not from negative one to one (which is default for OpenGL; see
///   glClipControl() for details).
template <typename T>
matrix<4, 4, T> make_perspective_projection_matrix(T fov_y, T aspect_ratio,
                                                   T near_plane,
                                                   T far_plane) noexcept
{
  const T h = 1 / std::tan(fov_y / 2);
  const T w = h / aspect_ratio;
  const T c = far_plane / (near_plane - far_plane);
  const T d = -(far_plane * near_plane) / (far_plane - near_plane);

  // clang-format off
  return make_matrix_from_row_major<4, 4, T>(
       w, T{0},  T{0}, T{0},
    T{0},    h,  T{0}, T{0},
    T{0}, T{0},     c,    d,
    T{0}, T{0}, T{-1}, T{0});
  // clang-format on
}

/// Returns an orthographic 4x4 projection matrix, which is used to transform
/// points from view space into clip space.
/// @remarks
///   Note that clip space is supposed to range from zero to one in z direction,
///   and not from negative one to one (which is default for OpenGL; see
///   glClipControl() for details).
template <typename T>
matrix<4, 4, T> make_orthographic_projection_matrix(T left, T right, T bottom,
                                                    T top) noexcept
{
  const T w = 2 / (right - left);
  const T h = 2 / (top - bottom);
  const T d = -(right + left) / (right - left);
  const T e = -(top + bottom) / (top - bottom);

  // clang-format off
  return make_matrix_from_row_major<4, 4, T>(
       w, T{0}, T{0},    d,
    T{0},    h, T{0},    e,
    T{0}, T{0}, T{1}, T{0},
    T{0}, T{0}, T{0}, T{1});
  // clang-format on
}

/// Returns an orthographic 4x4 projection matrix, which is used to transform
/// points from view space into clip space.
/// @remarks
///   Note that clip space is supposed to range from zero to one in z direction,
///   and not from negative one to one (which is default for OpenGL; see
///   glClipControl() for details).
template <typename T>
matrix<4, 4, T> make_orthographic_projection_matrix(T left, T right, T bottom,
                                                    T top, T near_plane,
                                                    T far_plane) noexcept
{
  T w = 2 / (right - left);
  T h = 2 / (top - bottom);
  T c = -1 / (far_plane - near_plane);
  T d = -(right + left) / (right - left);
  T e = -(top + bottom) / (top - bottom);
  T f = -near_plane / (far_plane - near_plane);

  // clang-format off
  return make_matrix_from_row_major<4, 4, T>(
       w, T{0}, T{0},    d,
    T{0},    h, T{0},    e,
    T{0}, T{0},    c,    f,
    T{0}, T{0}, T{0}, T{1});
  // clang-format on
}
}

#endif
