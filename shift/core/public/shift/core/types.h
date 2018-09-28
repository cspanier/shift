#ifndef SHIFT_CORE_TYPES_H
#define SHIFT_CORE_TYPES_H

#include <typeinfo>
#include <type_traits>
#include <memory>
#include <tuple>
#include <string>
#include <vector>
#include <map>
#include "shift/core/core.h"
#include "shift/core/recursive_wrapper.h"

namespace shift::core
{
template <typename, typename>
struct append_to_type_sequence;

/// Adds a type T to type TT<Ts...> resulting in type TT<Ts..., T>.
template <typename T, typename... Ts, template <typename...> class TT>
struct append_to_type_sequence<T, TT<Ts...>>
{
  using Type = TT<Ts..., T>;
};

/// Generates a type TT with N times T template arguments.
template <template <typename...> class TT, typename T, std::size_t N>
struct repeat
{
  using Type =
    typename append_to_type_sequence<T,
                                     typename repeat<TT, T, N - 1>::Type>::Type;
};

/// A partial specialization of repeat<T, N, TT> to end the recursion.
template <typename T, template <typename...> class TT>
struct repeat<TT, T, 0>
{
  using Type = TT<>;
};

namespace detail
{
  /// A trivial helper type useful for use with std::enable_if.
  enum class enabler
  {
  };

  constexpr enabler enabler_instance = {};
}

#define ENABLE_IF(Condition)                                      \
  std::enable_if_t<(Condition), ::shift::core::detail::enabler> = \
    ::shift::core::detail::enabler_instance
#define DISABLE_IF(Condition)                                      \
  std::enable_if_t<!(Condition), ::shift::core::detail::enabler> = \
    ::shift::core::detail::enabler_instance
#define ENABLE_IF_DEF(Condition) \
  std::enable_if_t<(Condition), ::shift::core::detail::enabler>
#define DISABLE_IF_DEF(Condition) \
  std::enable_if_t<!(Condition), ::shift::core::detail::enabler>

/// Tests if type T is a scoped enum type.
template <typename T>
using is_scoped_enum = std::conditional_t<
  std::is_enum<T>::value &&
    !std::is_convertible<T, std::underlying_type_t<T>>::value,
  std::true_type, std::false_type>;

/// Tests if all types are same.
template <typename... Ts>
struct are_same
{
  static constexpr bool value = true;
};

template <typename T1, typename T2>
struct are_same<T1, T2>
{
  static constexpr bool value = std::is_same<T1, T2>::value;
};

template <typename T1, typename T2, typename... Ts>
struct are_same<T1, T2, Ts...>
{
  static constexpr bool value =
    std::is_same<T1, T2>::value && are_same<T1, Ts...>::value;
};

template <typename... Ts>
constexpr bool are_same_v = are_same<Ts...>::value;

/// Similar to std::remove_reference, but only removes lvalue references.
template <typename T>
struct remove_lvalue_reference
{
  using type = T;
};

template <typename T>
struct remove_lvalue_reference<T&>
{
  using type = T;
};

///
template <typename T>
using remove_lvalue_reference_t = typename remove_lvalue_reference<T>::type;

/// Similar to std::remove_reference, but only removes rvalue references.
template <typename T>
struct remove_rvalue_reference
{
  using type = T;
};

template <typename T>
struct remove_rvalue_reference<T&&>
{
  using type = T;
};

///
template <typename T>
using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;

template <typename T>
using remove_ref_cv_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
class group_ptr;

/// Utility type to check for group_ptr<U> types.
template <typename T>
struct is_group_ptr : public std::false_type
{
};

/// Partial specialization for the case where type T matches group_ptr<U>.
template <typename U>
struct is_group_ptr<group_ptr<U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_group_ptr_v = is_group_ptr<T>::value;

/// Utility type to check for std::shared_ptr<T> types.
template <typename T>
struct is_std_shared_ptr : public std::false_type
{
};

/// Partial specialization for the case where type T matches
/// std::shared_ptr<U>.
template <typename U>
struct is_std_shared_ptr<std::shared_ptr<U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_std_shared_ptr_v = is_std_shared_ptr<T>::value;

/// Utility type to check for std::unique_ptr<T> types.
template <typename T>
struct is_std_unique_ptr : public std::false_type
{
};

/// Partial specialization for the case where type T matches
/// std::unique_ptr<U>.
template <typename U>
struct is_std_unique_ptr<std::unique_ptr<U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_std_unique_ptr_v = is_std_unique_ptr<T>::value;

/// Utility type to check for std::string types.
template <typename T>
struct is_std_string : public std::false_type
{
};

/// Partial specialization for the case where type T matches std::string.
template <>
struct is_std_string<std::string> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_std_string_v = is_std_string<T>::value;

/// Utility type to check for std::tuple<Ts...> types.
template <typename T>
struct is_std_tuple : public std::false_type
{
};

/// Partial specialization for the case where type T matches std::tuple<Ts...>.
template <typename... Ts>
struct is_std_tuple<std::tuple<Ts...>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_std_tuple_v = is_std_tuple<T>::value;

/// Utility type to check for std::vector<T> types.
template <typename T>
struct is_std_vector : public std::false_type
{
};

/// Partial specialization for the case where type T matches std::vector<U>.
template <typename U>
struct is_std_vector<std::vector<U>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_std_vector_v = is_std_vector<T>::value;

/// Utility type to check for std::map<T, U> types.
template <typename T>
struct is_std_map : public std::false_type
{
};

/// Partial specialization for the case where type T matches std::map<U, V>.
template <typename U, typename V>
struct is_std_map<std::map<U, V>> : public std::true_type
{
};

///
template <typename T>
constexpr bool is_std_map_v = is_std_map<T>::value;

template <typename T, typename = void>
struct is_iterator : public std::false_type
{
};

template <typename T>
struct is_iterator<
  T, std::enable_if_t<!std::is_same<
       typename std::iterator_traits<T>::value_type, void>::value>>
: public std::true_type
{
};

///
template <typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;

namespace detail
{
  template <std::size_t... Indices, typename... Ts, typename Visitor>
  inline void for_each_element(std::index_sequence<Indices...> /*unused*/,
                               std::tuple<Ts...>& elements, Visitor&& visitor)
  {
    ((void)visitor(std::get<Indices>(elements)), ...);
  }
}

///
template <typename... Ts, typename Visitor>
inline void for_each_element(std::tuple<Ts...>& elements, Visitor&& visitor)
{
  detail::for_each_element(std::index_sequence_for<Ts...>{}, elements,
                           std::forward<Visitor>(visitor));
}

/// Quick support type which transforms any compile-time constant of type
/// size_t to type T.
/// @remarks
///   This is particularly useful for processing a std::index_sequence.
template <std::size_t, typename T>
using index_to_type = T;

/// Returns the first type of a parameter pack.
template <typename T, typename... Ts>
struct first_in_pack
{
  using type = T;
};

template <typename... Ts>
using first_in_pack_t = typename first_in_pack<Ts...>::type;

/// Returns the address of a rvalue reference.
/// @remarks
///   This is used to silence a GCC warning about taking the address of a
///   temporary.
template <typename T>
const T* rvalue_address(const T& value)
{
  return &value;
}
}

#endif
