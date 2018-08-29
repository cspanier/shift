#ifndef SHIFT_MATH_MATH_H
#define SHIFT_MATH_MATH_H

#include <cstddef>

#if defined(SHIFT_MATH_SHARED)
#if defined(SHIFT_MATH_EXPORT)
#define SHIFT_MATH_API SHIFT_EXPORT
#else
#define SHIFT_MATH_API SHIFT_IMPORT
#endif
#else
#define SHIFT_MATH_API
#endif

namespace shift::math
{
struct do_not_initialize
{
};

template <std::size_t Rows, std::size_t Columns, typename T>
class matrix;
template <typename T>
using matrix22 = matrix<2, 2, T>;
template <typename T>
using matrix33 = matrix<3, 3, T>;
template <typename T>
using matrix44 = matrix<4, 4, T>;

template <std::size_t Rows, typename T>
using vector = matrix<Rows, 1, T>;
template <typename T>
using vector1 = vector<1, T>;
template <typename T>
using vector2 = vector<2, T>;
template <typename T>
using vector3 = vector<3, T>;
template <typename T>
using vector4 = vector<4, T>;

template <typename T>
class quaternion;

template <std::size_t N, typename T>
class ray;
template <typename T>
using ray2 = ray<2, T>;
template <typename T>
using ray3 = ray<3, T>;
template <typename T>
using ray4 = ray<4, T>;

template <std::size_t N, typename T>
class line;
template <typename T>
using line2 = line<2, T>;
template <typename T>
using line3 = line<3, T>;
template <typename T>
using line4 = line<4, T>;

template <std::size_t N, typename T>
class plane;
template <typename T>
using plane2 = plane<2, T>;
template <typename T>
using plane3 = plane<3, T>;
template <typename T>
using plane4 = plane<4, T>;

template <std::size_t N, typename T>
class sphere;
template <typename T>
using sphere2 = sphere<2, T>;
template <typename T>
using sphere3 = sphere<3, T>;
template <typename T>
using sphere4 = sphere<4, T>;

template <std::size_t Dimensions, typename T>
class interval;
template <typename T>
using rectangle = interval<2, T>;
template <typename T>
using interval2 = interval<2, T>;
template <typename T>
using interval3 = interval<3, T>;
template <typename T>
using interval4 = interval<4, T>;

template <std::size_t N, typename T>
class frustum;
template <typename T>
using frustum2 = frustum<2, T>;
template <typename T>
using frustum3 = frustum<3, T>;
template <typename T>
using frustum4 = frustum<4, T>;

/// Utility type to check for vector<N, T> types.
template <typename T>
struct is_vector : public std::false_type
{
};

/// Partial specialization for the case where type T matches vector<N, U>.
template <std::size_t N, typename U>
struct is_vector<math::vector<N, U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_vector_v = is_vector<T>::value;

/// Utility type to check for vector2<T> types.
template <typename T>
struct is_vector2 : public std::false_type
{
};

/// Partial specialization for the case where type T matches vector2<U>.
template <typename U>
struct is_vector2<math::vector2<U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_vector2_v = is_vector2<T>::value;

/// Utility type to check for vector3<T> types.
template <typename T>
struct is_vector3 : public std::false_type
{
};

/// Partial specialization for the case where type T matches vector3<U>.
template <typename U>
struct is_vector3<math::vector3<U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_vector3_v = is_vector3<T>::value;

/// Utility type to check for vector4<T> types.
template <typename T>
struct is_vector4 : public std::false_type
{
};

/// Partial specialization for the case where type T matches vector4<U>.
template <typename U>
struct is_vector4<math::vector4<U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_vector4_v = is_vector4<T>::value;

/// Utility type to check for matrix<N, T> types.
template <typename T>
struct is_matrix : public std::false_type
{
};

/// Partial specialization for the case where type T matches
/// matrix<Rows, Columns, U>.
template <std::size_t Rows, std::size_t Columns, typename U>
struct is_matrix<math::matrix<Rows, Columns, U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_matrix_v = is_matrix<T>::value;

/// Utility type to check for matrix<2, 2, T> types.
template <typename T>
struct is_matrix22 : public std::false_type
{
};

/// Partial specialization for the case where type T matches
/// matrix<2, 2, U>.
template <typename U>
struct is_matrix22<math::matrix<2, 2, U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_matrix22_v = is_matrix22<T>::value;

/// Utility type to check for matrix<3, 3, T> types.
template <typename T>
struct is_matrix33 : public std::false_type
{
};

/// Partial specialization for the case where type T matches
/// matrix<3, 3, U>.
template <typename U>
struct is_matrix33<math::matrix<3, 3, U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_matrix33_v = is_matrix33<T>::value;

/// Utility type to check for matrix<4, 4, T> types.
template <typename T>
struct is_matrix44 : public std::false_type
{
};

/// Partial specialization for the case where type T matches
/// matrix<4, 4, U>.
template <typename U>
struct is_matrix44<math::matrix<4, 4, U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_matrix44_v = is_matrix44<T>::value;
}

#endif
