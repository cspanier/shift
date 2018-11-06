#ifndef SHIFT_MATH_MATH_HPP
#define SHIFT_MATH_MATH_HPP

#include <cstddef>
#include <type_traits>

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
template <std::size_t Rows, typename T>
struct vector;
template <typename T>
using vector2 = vector<2, T>;
template <typename T>
using vector3 = vector<3, T>;
template <typename T>
using vector4 = vector<4, T>;

template <std::size_t Rows, std::size_t Columns, typename T>
struct matrix;
template <typename T>
using matrix22 = matrix<2, 2, T>;
template <typename T>
using matrix33 = matrix<3, 3, T>;
template <typename T>
using matrix44 = matrix<4, 4, T>;

struct do_not_initialize
{
};

/// A simple tag type used to select argument order in matrix construction.
struct column_major
{
};

/// A simple tag type used to select argument order in matrix construction.
struct row_major
{
};

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

namespace shift::math::detail
{
///
template <typename... Ts>
struct components_count
{
  static constexpr std::size_t count()
  {
    return 0;
  }
};

///
template <typename T, typename... Ts>
struct components_count<T, Ts...>
{
  static constexpr std::size_t count()
  {
    if constexpr (is_vector_v<T>)
      return T::row_count + components_count<Ts...>::count();
    else
      return 1 + components_count<Ts...>::count();
  }
};

///
template <typename... Ts>
constexpr std::size_t components_count_v = components_count<Ts...>::count();

///
template <typename T>
struct component_type
{
  using type = T;
};

///
template <std::size_t Rows, typename T>
struct component_type<math::vector<Rows, T>>
{
  using type = T;
};

///
template <typename T>
using component_type_t = typename component_type<T>::type;

///
template <typename... Ts>
struct select_type;
///
template <typename T>
struct select_type<T>
{
  using type = component_type_t<T>;
};

///
template <typename T, typename... Ts>
struct select_type<T, Ts...>
{
  using type = decltype(std::declval<component_type_t<T>>() *
                        std::declval<typename select_type<Ts...>::type>());
};

///
template <typename... Ts>
using select_type_t = typename select_type<Ts...>::type;
}

#endif
