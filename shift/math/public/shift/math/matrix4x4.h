#ifndef SHIFT_MATH_MATRIX4X4_H
#define SHIFT_MATH_MATRIX4X4_H

#include <cstdint>
#include <cmath>
#include <utility>
#include "shift/math/math.h"
#include "shift/math/vector.h"

namespace shift::math
{
template <typename T>
struct matrix<4, 4, T>
{
  static constexpr std::size_t row_count = 4;
  static constexpr std::size_t column_count = 4;

  using value_type = T;
  using column_type = vector<row_count, T>;
  using row_type = vector<column_count, T>;

  constexpr matrix() noexcept = default;

  /// Constructs a matrix and assigns each element the same value fill_value.
  constexpr matrix(T fill_value) noexcept;

  /// Constructs the matrix from a series from column major ordered values.
  constexpr matrix(column_major, T value00, T value10, T value20, T value30,
                   T value01, T value11, T value21, T value31, T value02,
                   T value12, T value22, T value32, T value03, T value13,
                   T value23, T value33) noexcept;

  /// Constructs the matrix from a series from row major ordered values.
  constexpr matrix(row_major, T value00, T value01, T value02, T value03,
                   T value10, T value11, T value12, T value13, T value20,
                   T value21, T value22, T value23, T value30, T value31,
                   T value32, T value33) noexcept;

  /// Constructs the matrix from column vectors.
  constexpr matrix(column_major, const column_type& column0,
                   const column_type& column1, const column_type& column2,
                   const column_type& column3) noexcept;

  /// Constructs the matrix from row vectors.
  constexpr matrix(row_major, const row_type& row0, const row_type& row1,
                   const row_type& row2, const row_type& row3) noexcept;

  /// Construct the matrix from a one dimensional C-style array of values stored
  /// in column major order.
  constexpr matrix(column_major,
                   const T (&array)[row_count * column_count]) noexcept;

  /// Construct the matrix from a one dimensional C-style array of values stored
  /// in row major order.
  constexpr matrix(row_major,
                   const T (&array)[row_count * column_count]) noexcept;

  /// Construct the matrix from a one dimensional std::array of values stored in
  /// column major order.
  constexpr matrix(
    column_major,
    const std::array<T, row_count * column_count>& array) noexcept;

  /// Construct the matrix from a one dimensional std::array of values stored in
  /// row major order.
  constexpr matrix(
    row_major, const std::array<T, row_count * column_count>& array) noexcept;

  /// @pre
  ///   The number of elements between begin and end must equal the number of
  ///   elements of this matrix.
  template <typename Iterator>
  constexpr matrix(Iterator begin, Iterator end) noexcept SHIFT_EXPECTS(
    std::distance(begin, end) == row_count * column_count);

  constexpr matrix(const matrix&) noexcept = default;
  constexpr matrix(matrix&&) noexcept = default;
  ~matrix() noexcept = default;
  constexpr matrix& operator=(const matrix&) noexcept = default;
  constexpr matrix& operator=(matrix&&) noexcept = default;

  /// Selects a single matrix element by row and column.
  /// @pre
  ///   The row selector must not exceed the number of rows.
  /// @pre
  ///   The column selector must not exceed the number of columns.
  constexpr T& operator()(std::size_t row, std::size_t column)
    SHIFT_EXPECTS(row < row_count && column < column_count);

  /// Selects a single matrix element by row and column.
  /// @pre
  ///   The row selector must not exceed the number of rows.
  /// @pre
  ///   The column selector must not exceed the number of columns.
  constexpr T operator()(std::size_t row, std::size_t column) const
    SHIFT_EXPECTS(row < row_count && column < column_count);

  /// Returns a column vector.
  constexpr column_type& column_vector(std::size_t column);
  /// Returns a column vector.
  constexpr const column_type& column_vector(std::size_t column) const;
  /// Returns a row vector.
  constexpr column_type row_vector(std::size_t row) const;

  column_type _columns[column_count];
};

/// Performs matrix-vector multiplication.
template <typename U, typename V>
constexpr auto operator*(const matrix<4, 4, U>& lhs, const vector<4, V>& rhs)
{
  return vector<4, decltype(std::declval<U>() * std::declval<V>())>{
    dot(lhs.row_vector(0), rhs), dot(lhs.row_vector(1), rhs),
    dot(lhs.row_vector(2), rhs), dot(lhs.row_vector(3), rhs)};
}

/// Element-wise matrix-matrix division.
template <typename U, typename V>
constexpr auto operator/(const matrix<4, 4, U>& lhs, const matrix<4, 4, V>& rhs)
{
  // clang-format off
  return matrix<4, 4, decltype(std::declval<U>() / std::declval<V>())>{
    column_major{},
    lhs(0, 0) / rhs(0, 0), lhs(1, 0) / rhs(1, 0),
    lhs(2, 0) / rhs(2, 0), lhs(3, 0) / rhs(3, 0),
    lhs(0, 1) / rhs(0, 1), lhs(1, 1) / rhs(1, 1),
    lhs(2, 1) / rhs(2, 1), lhs(3, 1) / rhs(3, 1),
    lhs(0, 2) / rhs(0, 2), lhs(1, 2) / rhs(1, 2),
    lhs(2, 2) / rhs(2, 2), lhs(3, 2) / rhs(3, 2),
    lhs(0, 3) / rhs(0, 3), lhs(1, 3) / rhs(1, 3),
    lhs(2, 3) / rhs(2, 3), lhs(3, 3) / rhs(3, 3)};
  // clang-format on
}

/// Element-wise matrix-scalar division.
template <typename U, typename V, std::enable_if_t<!is_matrix_v<V>>* = nullptr>
constexpr auto operator/(const matrix<4, 4, U>& lhs, V rhs)
{
  // clang-format off
  return matrix<4, 4, decltype(std::declval<U>() / std::declval<V>())>{
    column_major{},
    lhs(0, 0) / rhs, lhs(1, 0) / rhs, lhs(2, 0) / rhs, lhs(3, 0) / rhs,
    lhs(0, 1) / rhs, lhs(1, 1) / rhs, lhs(2, 1) / rhs, lhs(3, 1) / rhs,
    lhs(0, 2) / rhs, lhs(1, 2) / rhs, lhs(2, 2) / rhs, lhs(3, 2) / rhs,
    lhs(0, 3) / rhs, lhs(1, 3) / rhs, lhs(2, 3) / rhs, lhs(3, 3) / rhs};
  // clang-format on
}

/// Returns the determinant of a 3x3 matrix.
template <typename T>
constexpr T determinant(const matrix<4, 4, T>& any_matrix) noexcept
{
  return any_matrix(0, 0) * (any_matrix(1, 1) * any_matrix(2, 2) -
                             any_matrix(2, 1) * any_matrix(1, 2)) -
         any_matrix(0, 1) * (any_matrix(1, 0) * any_matrix(2, 2) -
                             any_matrix(2, 0) * any_matrix(1, 2)) +
         any_matrix(0, 2) * (any_matrix(1, 0) * any_matrix(2, 1) -
                             any_matrix(2, 0) * any_matrix(1, 1));
}

/// Returns the inverse of a 3x3 matrix.
template <typename T>
constexpr auto inverse(const matrix<4, 4, T>& any_matrix) noexcept
{
  return matrix<4, 4, T>{column_major{},
                         +determinant(sub_matrix(any_matrix, 0, 0)),
                         -determinant(sub_matrix(any_matrix, 0, 1)),
                         +determinant(sub_matrix(any_matrix, 0, 2)),
                         -determinant(sub_matrix(any_matrix, 0, 3)),
                         -determinant(sub_matrix(any_matrix, 1, 0)),
                         +determinant(sub_matrix(any_matrix, 1, 1)),
                         -determinant(sub_matrix(any_matrix, 1, 2)),
                         +determinant(sub_matrix(any_matrix, 1, 3)),
                         +determinant(sub_matrix(any_matrix, 2, 0)),
                         -determinant(sub_matrix(any_matrix, 2, 1)),
                         +determinant(sub_matrix(any_matrix, 2, 2)),
                         -determinant(sub_matrix(any_matrix, 2, 3)),
                         -determinant(sub_matrix(any_matrix, 3, 0)),
                         +determinant(sub_matrix(any_matrix, 3, 1)),
                         -determinant(sub_matrix(any_matrix, 3, 2)),
                         +determinant(sub_matrix(any_matrix, 3, 3))} /
         determinant(any_matrix);
}

template <typename T>
constexpr matrix<4, 4, T>::matrix(T fill_value) noexcept
: _columns{{fill_value}, {fill_value}, {fill_value}, {fill_value}}
{
}

template <typename T>
constexpr matrix<4, 4, T>::matrix(column_major, T value00, T value10, T value20,
                                  T value30, T value01, T value11, T value21,
                                  T value31, T value02, T value12, T value22,
                                  T value32, T value03, T value13, T value23,
                                  T value33) noexcept
: _columns{{value00, value10, value20, value30},
           {value01, value11, value21, value31},
           {value02, value12, value22, value32},
           {value03, value13, value23, value33}}
{
}

template <typename T>
constexpr matrix<4, 4, T>::matrix(row_major, T value00, T value01, T value02,
                                  T value03, T value10, T value11, T value12,
                                  T value13, T value20, T value21, T value22,
                                  T value23, T value30, T value31, T value32,
                                  T value33) noexcept
: _columns{{value00, value10, value20, value30},
           {value01, value11, value21, value31},
           {value02, value12, value22, value32},
           {value03, value13, value23, value33}}
{
}

template <typename T>
constexpr matrix<4, 4, T>::matrix(
  column_major, const matrix<4, 4, T>::column_type& column0,
  const matrix<4, 4, T>::column_type& column1,
  const matrix<4, 4, T>::column_type& column2,
  const matrix<4, 4, T>::column_type& column3) noexcept
: _columns{column0, column1, column2, column3}
{
}

template <typename T>
constexpr matrix<4, 4, T>::matrix(
  row_major, const matrix<4, 4, T>::row_type& row0,
  const matrix<4, 4, T>::row_type& row1, const matrix<4, 4, T>::row_type& row2,
  const matrix<4, 4, T>::row_type& row3) noexcept
: _columns{{row0(0), row1(0), row2(0), row3(0)},
           {row0(1), row1(1), row2(1), row3(1)},
           {row0(2), row1(2), row2(2), row3(2)},
           {row0(3), row1(3), row2(3), row3(3)}}
{
}

template <typename T>
template <typename Iterator>
constexpr matrix<4, 4, T>::matrix(Iterator begin, Iterator end) noexcept
: _columns{{begin, begin + 4},
           {begin + 4, begin + 8},
           {begin + 8, begin + 12},
           {begin + 12, end}}
{
}

template <typename T>
constexpr T& matrix<4, 4, T>::operator()(std::size_t row, std::size_t column)
{
  BOOST_ASSERT(row < row_count);
  BOOST_ASSERT(column < column_count);
  return _columns[column](row);
}

template <typename T>
constexpr T matrix<4, 4, T>::operator()(std::size_t row,
                                        std::size_t column) const
{
  BOOST_ASSERT(row < row_count);
  BOOST_ASSERT(column < column_count);
  return _columns[column](row);
}

template <typename T>
constexpr typename matrix<4, 4, T>::column_type& matrix<4, 4, T>::column_vector(
  std::size_t column)
{
  return _columns[column];
}

template <typename T>
constexpr const typename matrix<4, 4, T>::column_type&
matrix<4, 4, T>::column_vector(std::size_t column) const
{
  return _columns[column];
}

template <typename T>
constexpr typename matrix<4, 4, T>::column_type matrix<4, 4, T>::row_vector(
  std::size_t row) const
{
  return make_vector_from(_columns[0](row), _columns[1](row), _columns[2](row),
                          _columns[3](row));
}
}

#endif
