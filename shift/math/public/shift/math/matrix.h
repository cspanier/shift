#ifndef SHIFT_MATH_MATRIX_H
#define SHIFT_MATH_MATRIX_H

#include <cmath>
#include <array>
#include <iostream>
#include <shift/core/types.h>
#include <shift/core/algorithm.h>
#include "shift/math/math.h"
#include "shift/math/utility.h"
#include "shift/math/detail/matrix_storage.h"

namespace shift::math::detail
{
struct column_major
{
};

struct row_major
{
};

template <typename... Ts>
struct count_elements
{
  static constexpr std::size_t count()
  {
    return 0;
  }
};

template <typename T, typename... Ts>
struct count_elements<T, Ts...>
{
  static constexpr std::size_t count()
  {
    if constexpr (is_matrix_v<T>)
      return T::rows * T::columns + count_elements<Ts...>::count();
    if constexpr (core::is_std_tuple<T>::value)
      return std::tuple_size_v<T> + count_elements<Ts...>::count();
    else
      return 1 + count_elements<Ts...>::count();
  }
};

template <typename T>
struct element_type
{
  using type = T;
};

template <std::size_t Rows, std::size_t Columns, typename T>
struct element_type<matrix<Rows, Columns, T>>
{
  using type = typename element_type<std::decay_t<T>>::type;
};

template <typename T, typename... Ts>
struct element_type<std::tuple<T, Ts...>>
{
  using type = typename element_type<std::decay_t<T>>::type;
};

template <typename T>
using element_type_t = typename element_type<T>::type;
}

namespace shift::math
{
/// Template class for T^(Columns*Rows) matrices in column major order.
template <std::size_t Rows, std::size_t Columns, typename T>
class matrix final : public detail::matrix_storage<Rows, Columns, T>
{
public:
  static_assert(Rows >= 1, "A matrix needs to have at least one row");
  static_assert(Columns >= 1, "A matrix needs to have at least one column");

  using base_t = detail::matrix_storage<Rows, Columns, T>;
  using element_t = T;

  static constexpr std::size_t rows = Rows;
  static constexpr std::size_t columns = Columns;

public:
  /// Default constructor.
  constexpr matrix() noexcept = default;

  /// Copy constructor.
  constexpr matrix(const matrix& /*other*/) noexcept = default;

  /// Move constructor.
  constexpr matrix(matrix&& /*other*/) noexcept = default;

  /// Constructor initializing the matrix from a list of values in column
  /// major order.
  template <typename... Us>
  constexpr matrix(detail::column_major, Us&&... values) noexcept;

  /// Constructor initializing the matrix from a list of values in row major
  /// order.
  template <typename... Us>
  constexpr matrix(detail::row_major, Us&&... values) noexcept;

  /// Constructor supporting brace-init-lists.
  template <typename... Us>
  constexpr matrix(Us... args) noexcept : base_t{args...}
  {
  }

  /// Copy assignment operator.
  matrix& operator=(const matrix& /*other*/) noexcept = default;

  /// Move assignment operator.
  constexpr matrix& operator=(matrix&& /*other*/) noexcept = default;

  /// Matrix element-wise addition.
  matrix& operator+=(const matrix& other) noexcept;

  /// Matrix element-wise subtraction.
  matrix& operator-=(const matrix& other) noexcept;

  /// Matrix multiplication.
  matrix& operator*=(const matrix& other) noexcept;

  /// Function call operator to access individual components.
  T& operator()(std::size_t row);

  /// Function call operator to access individual components.
  constexpr const T operator()(std::size_t row) const;

  /// Function call operator to access individual components.
  T& operator()(std::size_t row, std::size_t column);

  /// Function call operator to access individual components.
  constexpr const T operator()(std::size_t row, std::size_t column) const;

  /// Returns the address of the matrix data.
  T* data() noexcept;

  /// Returns the address of the matrix data.
  const T* data() const noexcept;

  /// Expand tuple values and assign them to individual data indices.
  template <typename Direction, std::size_t Index, std::size_t... Indices,
            typename U>
  constexpr void assign_tuple(std::index_sequence<Indices...>, U&& value)
  {
    assign<Direction, Index>(std::move(std::get<Indices>(value))...);
  }

  /// This end of recursion helper for assign(Ts...) is used only to check
  /// whether the number of total elements passed matches the number of
  /// matrix elements.
  template <typename Direction, std::size_t Index>
  constexpr void assign() noexcept
  {
    static_assert(Index == Rows * Columns,
                  "The number of passed valued does not match the number of "
                  "matrix elements.");
  }

  /// Recursively assign various values to this matrix.
  template <typename Direction, std::size_t Index, typename U, typename... Us>
  constexpr void assign(U value, Us... other_values) noexcept
  {
    if constexpr (is_matrix_v<U>)
    {
      constexpr auto count = U::rows * U::columns;
      for (std::size_t i = 0; i < count; ++i)
        base_t::array[index<Direction>(Index + i)] = std::move(value.array[i]);

      assign<Direction, Index + count, Us...>(
        std::forward<Us>(other_values)...);
    }
    else if constexpr (core::is_std_tuple_v<U>)
    {
      constexpr auto count = std::tuple_size_v<U>;
      assign_tuple<Direction, Index>(std::make_index_sequence<count>{},
                                     std::forward<U>(value));

      assign<Direction, Index + count, Us...>(
        std::forward<Us>(other_values)...);
    }
    else
    {
      // Assume that U is of scalar type.
      base_t::array[index<Direction>(Index)] = std::move(value);

      assign<Direction, Index + 1, Us...>(std::forward<Us>(other_values)...);
    }
  }

private:
  /// Helper function to conditionally transform row major matrix indices to
  /// column major order indices.
  template <typename Direction>
  constexpr std::size_t index(std::size_t index)
  {
    if constexpr (std::is_same_v<Direction, detail::column_major>)
      return index;
    else if constexpr (std::is_same_v<Direction, detail::row_major>)
    {
      auto row = index / Columns;
      auto column = index % Columns;
      return column * Rows + row;
    }
  }
};

/// Construct a matrix by copying fill_value to each element.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_default_matrix(T fill_value)
{
  matrix<Rows, Columns, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = fill_value;
  }
  return result;
}

/// Constructs a matrix filled with either signaling_nan<T>() or T{0}, depending
/// on whether type T does have a signalling NaN value defined.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr matrix<Rows, Columns, T> make_nan_matrix() noexcept
{
  return make_default_matrix<Rows, Columns, T>(
    std::numeric_limits<T>::has_signaling_NaN ? signaling_nan<T>() : T{0});
}

///
template <std::size_t Rows, std::size_t Columns, typename T, typename... Ts,
          std::enable_if_t<(sizeof...(Ts) > 1)>* = nullptr>
constexpr auto make_matrix_from_column_major(Ts&&... values) noexcept
{
  return matrix<Rows, Columns, T>(detail::column_major{},
                                  std::forward<Ts>(values)...);
}

///
template <std::size_t Rows, std::size_t Columns, typename T, typename... Ts,
          std::enable_if_t<(sizeof...(Ts) > 1)>* = nullptr>
constexpr auto make_matrix_from_row_major(Ts&&... values) noexcept
{
  return matrix<Rows, Columns, T>(detail::row_major{},
                                  std::forward<Ts>(values)...);
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
  matrix<Rows, Columns, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = array[column * Rows + row];
  }
  return result;
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
  matrix<Rows, Columns, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = array[row * Columns + column];
  }
  return result;
}

/// Construct a matrix from a std::array of values stored in column major
/// order.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_matrix_from_column_major(
  const std::array<T, Rows * Columns>& array) noexcept
{
  matrix<Rows, Columns, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = array[column * Rows + row];
  }
  return result;
}

/// Construct a matrix from a std::array of values stored in row major order.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_matrix_from_row_major(
  const std::array<T, Rows * Columns>& array) noexcept
{
  matrix<Rows, Columns, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = array[row * Columns + column];
  }
  return result;
}

/// Returns an identity matrix.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_identity_matrix() noexcept
{
  // The code used to directly assign
  // result(row, column) = row == column ? T{1} : T{0};
  // but MSVC++ (_MSC_VER 1911) doesn't unroll the loop.
  matrix<Rows, Columns, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = T{0};
  }
  for (std::size_t i = 0; i < std::min(Rows, Columns); ++i)
    result(i, i) = T{1};
  return result;
}

/// Returns a translation matrix.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_translation_matrix(const vector<Rows - 1, T>& v) noexcept
{
  matrix<Rows, Columns, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
    {
      result(row, column) = ((column == Columns - 1) && (row < Rows - 1))
                              ? v(row)
                              : (row == column ? T{1} : T{0});
    }
  }
  return result;
}

/// Returns a translation matrix.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr auto make_translation_matrix(const vector<Rows, T>& v) noexcept
{
  matrix<Rows, Columns, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
    {
      result(row, column) =
        (column == Columns - 1) ? v(row) : (row == column ? T{1} : T{0});
    }
  }
  return result;
}

/// Returns a translation matrix.
template <std::size_t Rows, std::size_t Columns, typename T, typename... Ts>
constexpr auto make_translation_matrix(Ts... axis) noexcept
{
  return make_translation_matrix<Rows, Columns>(
    make_matrix_from_column_major<sizeof...(Ts), 1, T>(
      static_cast<T>(axis)...));
}

/// Returns a 4x4 transformation matrix from a translation vector and a 3x3
/// rotation matrix.
template <typename T>
constexpr matrix<4, 4, T> make_transformation_matrix(
  const vector<3, T>& translation, const matrix<3, 3, T>& rotation) noexcept
{
  // clang-format off
  return make_matrix_from_row_major<4, 4, T>(
    rotation(0, 0), rotation(0, 1), rotation(0, 2), translation.x,
    rotation(1, 0), rotation(1, 1), rotation(1, 2), translation.y,
    rotation(2, 0), rotation(2, 1), rotation(2, 2), translation.z,
              T{0},           T{0},           T{0}, T{1});
  // clang-format on
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

/// Equality operator for floating point types.
template <std::size_t Rows, std::size_t Columns, typename T,
          std::enable_if_t<std::is_floating_point<T>::value>* = nullptr>
inline constexpr bool operator==(const matrix<Rows, Columns, T>& lhs,
                                 const matrix<Rows, Columns, T>& rhs) noexcept
{
  using std::abs;
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
    {
      if (abs(lhs(row, column) - rhs(row, column)) > epsilon<T>)
        return false;
    }
  }
  return true;
}

/// Equality operator for non floating point types.
template <std::size_t Rows, std::size_t Columns, typename T,
          std::enable_if_t<!std::is_floating_point<T>::value>* = nullptr>
inline constexpr bool operator==(const matrix<Rows, Columns, T>& lhs,
                                 const matrix<Rows, Columns, T>& rhs) noexcept
{
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
    {
      if (lhs(row, column) != rhs(row, column))
        return false;
    }
  }
  return true;
}

/// Equality operator.
template <std::size_t Rows1, std::size_t Columns1, std::size_t Rows2,
          std::size_t Columns2, typename T,
          std::enable_if_t<(Rows1 != Rows2 || Columns1 != Columns2)>* = nullptr>
inline constexpr bool operator==(
  const matrix<Rows1, Columns1, T>& /*lhs*/,
  const matrix<Rows2, Columns2, T>& /*rhs*/) noexcept
{
  return false;
}

/// Inequality operator.
template <std::size_t Rows, std::size_t Columns, typename T>
inline constexpr bool operator!=(const matrix<Rows, Columns, T>& lhs,
                                 const matrix<Rows, Columns, T>& rhs) noexcept
{
  return !(lhs == rhs);
}

/// Matrix-matrix multiplication
template <std::size_t Columns1Rows2, std::size_t Rows1, std::size_t Columns2,
          typename T, typename U, ENABLE_IF(Columns2 > 1)>
auto operator*(const matrix<Rows1, Columns1Rows2, T>& lhs,
               const matrix<Columns1Rows2, Columns2, U>& rhs)
{
  matrix<Rows1, Columns2, decltype(std::declval<T>() * std::declval<U>())>
    result{};
  for (std::size_t column = 0; column < Columns2; ++column)
  {
    for (std::size_t row = 0; row < Rows1; ++row)
    {
      result(row, column) =
        dot(row_vector(lhs, row), column_vector(rhs, column));
    }
  }
  return result;
}

/// Matrix-vector multiplication
template <std::size_t Columns1Rows2, std::size_t Rows1, typename T, typename U>
auto operator*(const matrix<Rows1, Columns1Rows2, T>& lhs,
               const vector<Columns1Rows2, U>& rhs)
{
  vector<Rows1, decltype(std::declval<T>() * std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows1; ++row)
    result(row) = dot(row_vector(lhs, row), rhs);
  return result;
}

/// Matrix-scalar multiplication.
template <std::size_t Rows, std::size_t Columns, typename T, typename U,
          std::enable_if_t<!is_matrix_v<U>>* = nullptr>
constexpr auto operator*(const matrix<Rows, Columns, T>& lhs, U rhs)
{
  matrix<Rows, Columns, decltype(std::declval<T>() * std::declval<U>())>
    result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = lhs(row, column) * rhs;
  }
  return result;
}

/// Scalar-matrix multiplication.
template <std::size_t Rows, std::size_t Columns, typename T, typename U,
          std::enable_if_t<!is_matrix_v<T>>* = nullptr>
inline constexpr auto operator*(T lhs, const matrix<Rows, Columns, U>& rhs)
{
  matrix<Rows, Columns, decltype(std::declval<T>() * std::declval<U>())>
    result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = lhs * rhs(row, column);
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

/// Matrix-scalar division.
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

/// Matrix-scalar division.
template <std::size_t Rows, std::size_t Columns, typename T, typename U,
          std::enable_if_t<!is_matrix_v<T>>* = nullptr>
constexpr auto operator/(T lhs, const matrix<Rows, Columns, U>& rhs)
{
  matrix<Rows, Columns, decltype(std::declval<T>() / std::declval<U>())>
    result{};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
      result(row, column) = lhs / rhs(row, column);
  }
  return result;
}

/// String serialization.
template <std::size_t Rows, std::size_t Columns, typename T>
inline std::ostream& operator<<(std::ostream& stream,
                                const math::matrix<Rows, Columns, T>& matrix)
{
  stream << "(";
  for (std::size_t column = 0; column < Columns; ++column)
  {
    if (column != 0)
      stream << ", ";
    stream << column_vector(matrix, column);
  }
  stream << ")";

  return stream;
}

/// String deserialization.
template <std::size_t Rows, std::size_t Columns, typename T>
inline std::istream& operator>>(std::istream& stream,
                                math::matrix<Rows, Columns, T>& matrix)
{
  stream.get();
  for (std::size_t i = 0; i < Columns; ++i)
  {
    if (i != 0)
      stream.get();
    if ((stream >> matrix(i)).fail())
      break;
  }
  stream.get();

  if (stream.fail())
    matrix = math::matrix<Rows, Columns, T>();

  return stream;
}

/// An alternative component accessor function particularly useful in
/// parameter pack expansion context.
template <std::size_t Row, std::size_t Column, std::size_t Rows,
          std::size_t Columns, typename T>
constexpr T get(const matrix<Rows, Columns, T>& m)
{
  return m(Row, Column);
}

/// Returns a reference to a single column vector.
template <std::size_t Rows, std::size_t Columns, typename T,
          ENABLE_IF(Columns > 1)>
vector<Rows, T>& column_vector(matrix<Rows, Columns, T>& matrix,
                               std::size_t column) noexcept
{
  return *reinterpret_cast<vector<Rows, T>*>(&matrix(0, column));
}

/// Returns a single column vector.
template <std::size_t Rows, std::size_t Columns, typename T,
          ENABLE_IF(Columns > 1)>
constexpr vector<Rows, T> column_vector(const matrix<Rows, Columns, T>& matrix,
                                        std::size_t column) noexcept
{
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = matrix(row, column);
  return result;
}

/// Returns a single row vector.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr vector<Columns, T> row_vector(const matrix<Rows, Columns, T>& matrix,
                                        std::size_t row) noexcept
{
  vector<Columns, T> result{};
  for (std::size_t column = 0; column < Columns; ++column)
    result(column) = matrix(row, column);
  return result;
}

/// Extracts the 3D translation vector from a transformation matrix.
template <typename T>
inline constexpr vector<3, T> translation3(
  const matrix<4, 4, T>& matrix) noexcept
{
  return make_matrix_from_column_major<3, 1, T>(matrix(0, 3), matrix(1, 3),
                                                matrix(2, 3)) /
         matrix(3, 3);
}

/// Extracts the 4D translation vector from a transformation matrix.
template <typename T>
inline constexpr vector<4, T> translation4(
  const matrix<4, 4, T>& matrix) noexcept
{
  return column_vector(matrix, 3);
}

///// Returns a scale matrix that scales the X, Y and Z axes.
///// @remarks
/////   This is only defined for 4x4 matrices.
// template <typename M, typename T, ENABLE_IF(is_matrix44<M>::value)>
// M scale(T factor)
//{
//  M result = identity<M>();
//  for (std::size_t i = 0; i < 3; ++i)
//    result(i, i) = static_cast<typename M::element_type>(factor);
//  return result;
//}

///// Returns a scale matrix that scales the X, Y and Z axes.
///// @remarks
/////   This is only defined for 4x4 matrices.
// template <typename M, typename T, ENABLE_IF(is_matrix44<M>::value)>
// M scale(const vector3<T>& factor)
//{
//  M result = identity<M>();
//  for (std::size_t i = 0; i < 3; ++i)
//    result(i, i) = static_cast<typename M::element_type>(factor(i));
//  return result;
//}

///
template <typename U, std::size_t Rows, std::size_t Columns, typename T>
inline constexpr auto static_cast_matrix(
  const matrix<Rows, Columns, T>& other) noexcept
{
  return matrix<Rows, Columns, U>(detail::column_major{}, other);
}

/// Returns a (Columns-1)x(Columns-1) sub-matrix of the argument by cutting
/// one row and one column.
/// @remarks
///   This method is only defined for matrices larger than 2x2.
template <std::size_t Rows, std::size_t Columns, typename T,
          ENABLE_IF((Columns > 2) && (Rows > 2))>
constexpr auto sub_matrix(const matrix<Rows, Columns, T>& any_matrix,
                          std::size_t cut_row, std::size_t cut_column)
{
  static_assert(
    (Columns > 2) && (Rows > 2),
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

/// Returns the determinant of a 2x2 matrix.
template <typename T>
constexpr auto determinant(const matrix<2, 2, T>& matrix) noexcept
{
  return matrix(0, 0) * matrix(1, 1) - matrix(1, 0) * matrix(0, 1);
}

/// Returns the determinant of a NxN matrix.
template <std::size_t Rows, std::size_t Columns, typename T>
constexpr T determinant(const matrix<Rows, Columns, T>& matrix) noexcept
{
  auto result = T{0};
  for (std::size_t column = 0; column < Columns; ++column)
  {
    result += (static_cast<int>(column % 2) * -2 + 1) * matrix(0, column) *
              determinant(sub_matrix(matrix, 0, column));
  }
  return result;
}

/// Returns the inverse of a 2x2 matrix.
template <typename T>
constexpr auto inverse(const matrix<2, 2, T>& matrix) noexcept
{
  return make_matrix_from_row_major<2, 2, T>(+matrix(1, 1), -matrix(0, 1),
                                             -matrix(1, 0), +matrix(0, 0)) /
         determinant(matrix);
}

/// Returns the inverse of an NxN matrix.
template <std::size_t Rows, std::size_t Columns, typename T,
          ENABLE_IF(Rows > 2 && Columns > 2)>
constexpr auto inverse(const matrix<Rows, Columns, T>& any_matrix) noexcept
{
  matrix<Columns, Rows, T> result{};
  auto inv_det = 1 / determinant(any_matrix);
  for (std::size_t column = 0; column < Columns; ++column)
  {
    for (std::size_t row = 0; row < Rows; ++row)
    {
      result(row, column) = (static_cast<int>((row + column + 1) % 2) * 2 - 1) *
                            inv_det *
                            determinant(sub_matrix(any_matrix, column, row));
    }
  }
  return result;
}

template <std::size_t Rows, std::size_t Columns, typename T>
template <typename... Us>
constexpr matrix<Rows, Columns, T>::matrix(detail::column_major,
                                           Us&&... values) noexcept
: base_t{}
{
  assign<detail::column_major, 0, std::decay_t<Us>...>(values...);
}

template <std::size_t Rows, std::size_t Columns, typename T>
template <typename... Us>
constexpr matrix<Rows, Columns, T>::matrix(detail::row_major,
                                           Us&&... values) noexcept
: base_t{}
{
  assign<detail::row_major, 0, std::decay_t<Us>...>(values...);
}

template <std::size_t Rows, std::size_t Columns, typename T>
matrix<Rows, Columns, T>& matrix<Rows, Columns, T>::operator+=(
  const matrix<Rows, Columns, T>& other) noexcept
{
  *this = static_cast_matrix<T>(*this + other);
  return *this;
}

template <std::size_t Rows, std::size_t Columns, typename T>
matrix<Rows, Columns, T>& matrix<Rows, Columns, T>::operator-=(
  const matrix<Rows, Columns, T>& other) noexcept
{
  *this = static_cast_matrix<T>(*this - other);
  return *this;
}

template <std::size_t Rows, std::size_t Columns, typename T>
matrix<Rows, Columns, T>& matrix<Rows, Columns, T>::operator*=(
  const matrix<Rows, Columns, T>& other) noexcept
{
  *this = static_cast_matrix<T>(*this * other);
  return *this;
}

template <std::size_t Rows, std::size_t Columns, typename T>
T& matrix<Rows, Columns, T>::operator()(std::size_t row)
{
  return base_t::array[row];
}

template <std::size_t Rows, std::size_t Columns, typename T>
constexpr const T matrix<Rows, Columns, T>::operator()(std::size_t row) const
{
  return base_t::array[row];
}

template <std::size_t Rows, std::size_t Columns, typename T>
T& matrix<Rows, Columns, T>::operator()(std::size_t row, std::size_t column)
{
  return base_t::array[column * Rows + row];
}

template <std::size_t Rows, std::size_t Columns, typename T>
constexpr const T matrix<Rows, Columns, T>::operator()(std::size_t row,
                                                       std::size_t column) const
{
  return base_t::array[column * Rows + row];
}

template <std::size_t Rows, std::size_t Columns, typename T>
T* matrix<Rows, Columns, T>::data() noexcept
{
  return base_t::array.data();
}

template <std::size_t Rows, std::size_t Columns, typename T>
const T* matrix<Rows, Columns, T>::data() const noexcept
{
  return base_t::array.data();
}
}

#endif
