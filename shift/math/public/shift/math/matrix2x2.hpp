#ifndef SHIFT_MATH_MATRIX2X2_HPP
#define SHIFT_MATH_MATRIX2X2_HPP

#include <cstdint>
#include <cmath>
#include <utility>
#include "shift/math/math.hpp"
#include "shift/math/vector.hpp"

namespace shift::math
{
template <typename T>
struct matrix<2, 2, T>
{
  static constexpr std::size_t row_count = 2;
  static constexpr std::size_t column_count = 2;

  using value_type = T;
  using column_type = vector<row_count, T>;
  using row_type = vector<column_count, T>;

  constexpr matrix() noexcept = default;

  /// Constructs a matrix and assigns each element the same value fill_value.
  constexpr matrix(T fill_value) noexcept;

  /// Constructs the matrix from a series from column major ordered values.
  constexpr matrix(column_major /*unused*/, T value00, T value10, T value01,
                   T value11) noexcept;

  /// Constructs the matrix from a series from row major ordered values.
  constexpr matrix(row_major /*unused*/, T value00, T value01, T value10,
                   T value11) noexcept;

  /// Constructs the matrix from column vectors.
  constexpr matrix(column_major /*unused*/, const column_type& column0,
                   const column_type& column1) noexcept;

  /// Constructs the matrix from row vectors.
  constexpr matrix(row_major /*unused*/, const row_type& row0,
                   const row_type& row1) noexcept;

  /// Construct the matrix from a one dimensional C-style array of values stored
  /// in column major order.
  constexpr matrix(column_major /*unused*/,
                   const T (&array)[row_count * column_count]) noexcept;

  /// Construct the matrix from a one dimensional C-style array of values stored
  /// in row major order.
  constexpr matrix(row_major /*unused*/,
                   const T (&array)[row_count * column_count]) noexcept;

  /// Construct the matrix from a one dimensional std::array of values stored in
  /// column major order.
  constexpr matrix(
    column_major /*unused*/,
    const std::array<T, row_count * column_count>& array) noexcept;

  /// Construct the matrix from a one dimensional std::array of values stored in
  /// row major order.
  constexpr matrix(
    row_major /*unused*/,
    const std::array<T, row_count * column_count>& array) noexcept;

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

  /// Selects a single matrix element by index.
  /// @pre
  ///   The index selector must not exceed the number of elements.
  constexpr T& operator()(std::size_t index)
    SHIFT_EXPECTS(index < row_count * column_count);

  /// Selects a single matrix element by index.
  /// @pre
  ///   The index selector must not exceed the number of elements.
  constexpr T operator()(std::size_t index) const
    SHIFT_EXPECTS(index < row_count * column_count);

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
constexpr auto operator*(const matrix<2, 2, U>& lhs, const vector<2, V>& rhs)
{
  return vector<2, decltype(std::declval<U>() * std::declval<V>())>{
    dot(lhs.row_vector(0), rhs), dot(lhs.row_vector(1), rhs)};
}

/// Element-wise matrix-matrix division.
template <typename U, typename V>
constexpr auto operator/(const matrix<2, 2, U>& lhs, const matrix<2, 2, V>& rhs)
{
  return matrix<2, 2, decltype(std::declval<U>() / std::declval<V>())>{
    column_major{}, lhs(0, 0) / rhs(0, 0), lhs(1, 0) / rhs(1, 0),
    lhs(0, 1) / rhs(0, 1), lhs(1, 1) / rhs(1, 1)};
}

/// Element-wise matrix-scalar division.
template <typename U, typename V, std::enable_if_t<!is_matrix_v<V>>* = nullptr>
constexpr auto operator/(const matrix<2, 2, U>& lhs, V rhs)
{
  return matrix<2, 2, decltype(std::declval<U>() / std::declval<V>())>{
    column_major{}, lhs(0, 0) / rhs, lhs(1, 0) / rhs, lhs(0, 1) / rhs,
    lhs(1, 1) / rhs};
}

/// Returns the determinant of a 2x2 matrix.
template <typename T>
constexpr auto determinant(const matrix<2, 2, T>& any_matrix) noexcept
{
  return any_matrix(0, 0) * any_matrix(1, 1) -
         any_matrix(1, 0) * any_matrix(0, 1);
}

/// Returns the inverse of a 2x2 matrix.
template <typename T>
constexpr auto inverse(const matrix<2, 2, T>& any_matrix) noexcept
{
  return matrix<2, 2, T>(row_major{}, any_matrix(1, 1), -any_matrix(0, 1),
                         -any_matrix(1, 0), any_matrix(0, 0)) /
         determinant(any_matrix);
}

template <typename T>
constexpr matrix<2, 2, T>::matrix(T fill_value) noexcept
: _columns{{fill_value}, {fill_value}}
{
}

template <typename T>
constexpr matrix<2, 2, T>::matrix(column_major /*unused*/, T value00, T value10,
                                  T value01, T value11) noexcept
: _columns{{value00, value10}, {value01, value11}}
{
}

template <typename T>
constexpr matrix<2, 2, T>::matrix(row_major /*unused*/, T value00, T value01,
                                  T value10, T value11) noexcept
: _columns{{value00, value10}, {value01, value11}}
{
}

template <typename T>
constexpr matrix<2, 2, T>::matrix(
  column_major /*unused*/, const matrix<2, 2, T>::column_type& column0,
  const matrix<2, 2, T>::column_type& column1) noexcept
: _columns{column0, column1}
{
}

template <typename T>
constexpr matrix<2, 2, T>::matrix(
  row_major /*unused*/, const matrix<2, 2, T>::row_type& row0,
  const matrix<2, 2, T>::row_type& row1) noexcept
: _columns{{row0(0), row1(0)}, {row0(1), row1(1)}}
{
}

template <typename T>
constexpr matrix<2, 2, T>::matrix(
  column_major /*unused*/, const T (&array)[row_count * column_count]) noexcept
: _columns{{array[0], array[1]}, {array[2], array[3]}}
{
}

template <typename T>
constexpr matrix<2, 2, T>::matrix(
  row_major /*unused*/, const T (&array)[row_count * column_count]) noexcept
: _columns{{array[0], array[2]}, {array[1], array[3]}}
{
}

template <typename T>
constexpr matrix<2, 2, T>::matrix(
  column_major /*unused*/,
  const std::array<T, row_count * column_count>& array) noexcept
: _columns{{array[0], array[1]}, {array[2], array[3]}}
{
}

template <typename T>
constexpr matrix<2, 2, T>::matrix(
  row_major /*unused*/,
  const std::array<T, row_count * column_count>& array) noexcept
: _columns{{array[0], array[2]}, {array[1], array[3]}}
{
}

template <typename T>
template <typename Iterator>
constexpr matrix<2, 2, T>::matrix(Iterator begin, Iterator end) noexcept
: _columns{{begin, begin + 2}, {begin + 2, end}}
{
}

template <typename T>
constexpr T& matrix<2, 2, T>::operator()(std::size_t row, std::size_t column)
{
  BOOST_ASSERT(row < row_count);
  BOOST_ASSERT(column < column_count);
  return _columns[column](row);
}

template <typename T>
constexpr T matrix<2, 2, T>::operator()(std::size_t row,
                                        std::size_t column) const
{
  BOOST_ASSERT(row < row_count);
  BOOST_ASSERT(column < column_count);
  return _columns[column](row);
}

template <typename T>
constexpr T& matrix<2, 2, T>::operator()(std::size_t index)
{
  BOOST_ASSERT(index < row_count * column_count);
  return _columns[index / row_count](index % row_count);
}

template <typename T>
constexpr T matrix<2, 2, T>::operator()(std::size_t index) const
{
  BOOST_ASSERT(index < row_count * column_count);
  return _columns[index / row_count](index % row_count);
}

template <typename T>
constexpr typename matrix<2, 2, T>::column_type& matrix<2, 2, T>::column_vector(
  std::size_t column)
{
  return _columns[column];
}

template <typename T>
constexpr const typename matrix<2, 2, T>::column_type&
matrix<2, 2, T>::column_vector(std::size_t column) const
{
  return _columns[column];
}

template <typename T>
constexpr typename matrix<2, 2, T>::column_type matrix<2, 2, T>::row_vector(
  std::size_t row) const
{
  return make_vector_from(_columns[0](row), _columns[1](row));
}

/// An overload of make_matrix_from_column_major taking two column vectors.
/// @remarks
///   This overload has the benefit that the caller does not need to specify
///   template parameters explicitely.
template <typename T>
constexpr auto make_matrix_from_column_major(
  const vector<2, T>& column1, const vector<2, T>& column2) noexcept
{
  return matrix<2, 2, T>(column_major{}, column1, column2);
}

/// An overload of make_matrix_from_row_major taking two row vectors.
/// @remarks
///   This overload has the benefit that the caller does not need to specify
///   template parameters explicitely.
template <typename T>
constexpr auto make_matrix_from_row_major(const vector<2, T>& row1,
                                          const vector<2, T>& row2) noexcept
{
  return matrix<2, 2, T>(row_major{}, row1, row2);
}
}

#endif
