#ifndef SHIFT_CORE_MPL_HPP
#define SHIFT_CORE_MPL_HPP

#include <cstdint>
#include <type_traits>
#include <utility>
#include "shift/core/core.hpp"
#include "shift/core/types.hpp"

namespace shift::core
{
template <typename T1, typename T2>
struct pair;

template <typename... Ts>
struct vector;

template <typename... Ts>
struct set;

template <typename... Pairs>
struct multimap;

template <typename... Pairs>
struct map;

/// Gets the Nth type of the type container Container.
template <std::size_t N, typename Container>
struct get_type;

/// A shortcut type for get_type.
template <std::size_t N, typename Container>
using get_type_t = typename get_type<N, Container>::type;

/// Gets the Nth type of the type container Container, or Alternative if there
/// are not enough elements in Container.
template <std::size_t N, typename Container, typename Alternative = void>
struct get_type_opt;

/// A shortcut type for get_type_opt.
template <std::size_t N, typename Container, typename Alternative = void>
using get_type_opt_t = typename get_type_opt<N, Container, Alternative>::type;

/// Converts a container holding a single type T to that type.
template <typename Container>
struct single_type;

/// A shortcut type for single_type.
template <typename Container>
using single_type_t = typename single_type<Container>::type;

///
template <typename Container, typename T>
struct index_of;

///
template <typename Container, typename T>
struct index_of_optional;

/// Checks whether type T is in Container<Ts...>.
template <typename Container, typename T>
struct in;

/// Generates a Container<Vs...> from Container<Ts...> and Container<Us...>,
/// where each element V in Vs is also element of Ts and Us.
template <typename LContainer, typename RContainer>
struct intersection;

/// A shortcut type for intersection.
template <typename LContainer, typename RContainer>
using intersection_t = typename intersection<LContainer, RContainer>::type;

/// A support type which derives from std::true_type if the container type
/// Container is well-formed. Otherwise it derives from std::false_type.
template <class Container>
struct is_valid;

///
template <typename Container, typename T>
struct push_front;

/// A shortcut type for push_front.
template <typename Container, typename T>
using push_front_t = typename push_front<Container, T>::type;

///
template <typename Container, typename T>
struct push_back;

/// A shortcut type for push_back.
template <typename Container, typename T>
using push_back_t = typename push_back<Container, T>::type;

/// Inserts a type T into Container.
template <typename Container, typename T>
struct insert;

/// A shortcut type for insert.
template <typename Container, typename T>
using insert_t = typename insert<Container, T>::type;

namespace detail
{
  template <typename Order, typename LT, typename RT>
  using less = std::conditional_t<(index_of<Order, LT>::value <
                                   index_of<Order, RT>::value),
                                  std::true_type, std::false_type>;
}

///
template <typename Container, typename T, typename Order,
          template <typename, typename, typename> class Comparator =
            detail::less>
struct sorted_insert;

/// A shortcut type for sorted_insert.
template <typename Container, typename T, typename Order,
          template <typename, typename, typename> class Comparator =
            detail::less>
using sorted_insert_t =
  typename sorted_insert<Container, T, Order, Comparator>::type;

/// Concatenates elements of a series of containers into a single one.
template <typename... Containers>
struct concat;

/// A shortcut type for concat.
template <typename... Containers>
using concat_t = typename concat<Containers...>::type;

/// Transform Ts from Source container to Destination container.
/// @remarks
///   Pseudocode illustrating how this type works:
///   for (T : Ts...)
///     Destination := insert_t<Destination, Transform<T>::type>;
template <typename Source, typename Destination,
          template <typename Element> class Transform>
struct transform;

/// A shortcut type for transform.
template <typename Source, typename Destination,
          template <typename Element> class Transform>
using transform_t = typename transform<Source, Destination, Transform>::type;

/// Transform Ts from Source container to Destination container.
/// @remarks
///   Pseudocode illustrating how this type works:
///   for (T : Ts...)
///     Destination := Transform<Destination, T>::type;
template <typename Source, typename Destination,
          template <typename Container, typename Element> class Transform>
struct fold;

/// A shortcut type for fold.
template <typename Source, typename Destination,
          template <typename Container, typename Element> class Transform>
using fold_t = typename fold<Source, Destination, Transform>::type;

/// This type finds values within a map, which are associated with a key.
/// @tparam Map
///   The map to search.
/// @tparam Key
///   The key to search for.
template <typename Map, typename Key>
struct find;

/// A shortcut type for find.
template <typename Map, typename Key>
using find_t = typename find<Map, Key>::type;

/// This type swaps first and second types of a pair.
template <typename Pair>
struct reverse_pair;

/// A shortcut type for reverse_pair.
template <typename Pair>
using reverse_pair_t = typename reverse_pair<Pair>::type;

/// This type swaps key and value in each of the maps key value pairs. If the
/// passed map's values are not unique a multimap will be generated. Otherwise,
/// the resulting type will be a regular map.
template <typename Map>
struct reverse_map;

/// A shortcut type for reverse_map.
template <typename Map>
using reverse_map_t = typename reverse_map<Map>::type;

/// Transform Ts from Source container to Destination container.
/// @remarks
///   Pseudocode illustrating how this type works:
///   Destination := Sources<>;
///   for (T : Ts...)
///     Destination := if<Predicate<T>::value,
///                        concat_t<Destination, Sources<T>>,
///                        Destination>;
template <typename Source,
          template <typename Element, typename...> class Predicate,
          typename... Args>
struct select;

/// A shortcut type for select.
template <typename Source,
          template <typename Element, typename...> class Predicate,
          typename... Args>
using select_t = typename select<Source, Predicate, Args...>::type;

/// Creates a new set of unique types.
template <typename... Ts>
struct make_set;

/// A shortcut type for make_set.
template <typename... Ts>
using make_set_t = typename make_set<Ts...>::type;

/// Convert a type Container<Ts...> to std::tuple<Ts...>.
template <typename Container>
struct to_std_tuple;

/// A shortcut type for to_std_tuple.
template <typename Container>
using to_std_tuple_t = typename to_std_tuple<Container>::type;

/// Convert a type Container<Ts...> to vector<Ts...>.
template <typename Container>
struct to_vector;

/// A shortcut type for to_vector.
template <typename Container>
using to_vector_t = typename to_vector<Container>::type;

/// Convert a type Container<Ts...> to set<Ts...>.
template <typename Container>
struct to_set;

/// A shortcut type for to_set.
template <typename Container>
using to_set_t = typename to_set<Container>::type;

/// Used to loop through all elements of a container using the visitor
/// pattern.
template <typename Container>
struct for_each;

/// Used to loop through all elements of a container using the visitor
/// pattern.
template <typename Container>
struct recursion_helper;

/// A special empty type used in places where no type matches.
struct no_type
{
};

namespace detail
{
  template <template <typename...> class Sequence, typename... Ts>
  struct sequence_impl
  {
    using head = no_type;
    using tail = Sequence<>;
  };

  template <template <typename...> class Sequence, typename Head,
            typename... Ts>
  struct sequence_impl<Sequence, Head, Ts...>
  {
    using head = Head;
    using tail = Sequence<Ts...>;
  };

  template <template <typename...> class Sequence, typename... Ts>
  struct sequence_t : public sequence_impl<Sequence, Ts...>
  {
    static constexpr std::size_t size = sizeof...(Ts);
  };
}

///
template <typename T1, typename T2>
struct pair
{
  using first = T1;
  using second = T2;
};

template <typename T>
struct is_pair : std::false_type
{
};

template <typename T1, typename T2>
struct is_pair<pair<T1, T2>> : std::true_type
{
};

template <typename T>
constexpr bool is_pair_v = is_pair<T>::value;

/// Returns the first type of a pair.
template <typename Pair>
struct first;

template <typename First, typename Second>
struct first<pair<First, Second>>
{
  using type = First;
};

/// Returns the first type of a pair.
template <typename Pair>
struct second;

template <typename First, typename Second>
struct second<pair<First, Second>>
{
  using type = Second;
};

///
template <typename... Ts>
struct vector : public detail::sequence_t<vector, Ts...>
{
};

template <typename T>
struct is_vector : std::false_type
{
};

template <typename... Ts>
struct is_vector<vector<Ts...>> : std::true_type
{
};

template <typename T>
constexpr bool is_vector_v = is_vector<T>::value;

///
template <typename... Ts>
struct set : public detail::sequence_t<set, Ts...>
{
};

template <typename T>
struct is_set : std::false_type
{
};

template <typename... Ts>
struct is_set<set<Ts...>> : std::true_type
{
};

template <typename T>
constexpr bool is_set_v = is_set<T>::value;

/// A type map, which is organized as a vector of type pairs.
template <typename... Pairs>
struct multimap : public detail::sequence_t<multimap, Pairs...>
{
  static_assert((is_pair_v<Pairs> && ...));
};

template <typename T>
struct is_multimap : std::false_type
{
};

template <typename... Pairs>
struct is_multimap<multimap<Pairs...>> : std::true_type
{
};

template <typename T>
constexpr bool is_multimap_v = is_multimap<T>::value;

/// A type map, which is organized as a set of type pairs.
template <typename... Pairs>
struct map : public detail::sequence_t<map, Pairs...>
{
  static_assert((is_pair_v<Pairs> && ...));
};

template <typename T>
struct is_map : std::false_type
{
};

template <typename... Pairs>
struct is_map<map<Pairs...>> : std::true_type
{
};

template <typename T>
constexpr bool is_map_v = is_map<T>::value;

/// End of recursion.
template <typename T, template <typename...> class Container, typename... Ts>
struct get_type<0, Container<T, Ts...>>
{
  using type = T;
};

/// Recursively run through Ts until N is zero.
template <std::size_t N, template <typename...> class Container, typename T,
          typename... Ts>
struct get_type<N, Container<T, Ts...>>
{
  static_assert((N < 1 + sizeof...(Ts)),
                "N is larger than the number of elements in Container");
  using type = typename get_type<N - 1, Container<Ts...>>::type;
};

/// Special case for empty containers.
template <std::size_t N, template <typename...> class Container>
struct get_type<N, Container<>>
{
  // Trick the compiler not to evaluate the expression until this
  // specialization of get_type is used.
  static_assert(N && false, "Cannot get the Nth type of an empty container");
  using type = void;
};

/// End of recursion.
template <typename T, template <typename...> class Container, typename... Ts,
          typename Alternative>
struct get_type_opt<0, Container<T, Ts...>, Alternative>
{
  using type = T;
};

/// Recursively run through Ts until N is zero.
template <std::size_t N, template <typename...> class Container, typename T,
          typename... Ts, typename Alternative>
struct get_type_opt<N, Container<T, Ts...>, Alternative>
{
  using type =
    typename get_type_opt<N - 1, Container<Ts...>, Alternative>::type;
};

/// Special case for empty containers.
template <std::size_t N, template <typename...> class Container,
          typename Alternative>
struct get_type_opt<N, Container<>, Alternative>
{
  using type = Alternative;
};

template <template <typename...> class Container>
struct single_type<Container<>>
{
  static_assert(Container<>::size != 0,
                "Can't get element out of empty container.");
};

template <template <typename...> class Container, typename T, typename... Ts>
struct single_type<Container<T, Ts...>>
{
  static_assert(sizeof...(Ts) == 0,
                "Can't get single element out of container holding many types");
  using type = T;
};

namespace detail
{
  ///
  template <std::size_t Index, typename T, typename... Ts>
  struct index_of_impl;

  template <std::size_t Index, typename T, typename... Ts>
  struct index_of_impl<Index, T, T, Ts...>
  {
    static constexpr std::int32_t value = Index;
  };

  template <std::size_t Index, typename T, typename... Ts>
  struct index_of_impl<Index, T, recursive_wrapper<T>, Ts...>
  {
    static constexpr std::int32_t value = Index;
  };

  template <std::size_t Index, typename T, typename U, typename... Ts>
  struct index_of_impl<Index, T, U, Ts...>
  {
    static_assert(sizeof...(Ts) > 0, "Type T doesn't exist in type pack Ts.");
    static constexpr std::int32_t value =
      index_of_impl<Index + 1, T, Ts...>::value;
  };
}

/// Specialization of index_of for container type vector.
template <typename... Ts, typename T>
struct index_of<vector<Ts...>, T>
{
  static constexpr std::size_t value =
    detail::index_of_impl<0, T, Ts...>::value;
};

/// Specialization of index_of for container type set.
template <typename... Ts, typename T>
struct index_of<set<Ts...>, T>
{
  static constexpr std::size_t value =
    detail::index_of_impl<0, T, Ts...>::value;
};

/// Specialization of index_of for container type multimap.
/// @remarks
///   If Key exists multiple times then only the first index is returned.
template <typename... Pairs, typename Key>
struct index_of<multimap<Pairs...>, Key>
{
  static constexpr std::size_t value =
    index_of<transform_t<multimap<Pairs...>, set<>, first>, Key>::value;
};

/// Specialization of index_of for container type map.
template <typename... Pairs, typename Key>
struct index_of<map<Pairs...>, Key>
{
  static constexpr std::size_t value =
    index_of<transform_t<map<Pairs...>, set<>, first>, Key>::value;
};

namespace detail
{
  ///
  template <std::int32_t Index, typename T, typename... Ts>
  struct index_of_optional_impl;

  template <std::int32_t Index, typename T>
  struct index_of_optional_impl<Index, T>
  {
    static constexpr std::int32_t value = -1;
  };

  template <std::int32_t Index, typename T, typename... Ts>
  struct index_of_optional_impl<Index, T, T, Ts...>
  {
    static constexpr std::int32_t value = Index;
  };

  template <std::int32_t Index, typename T, typename... Ts>
  struct index_of_optional_impl<Index, T, recursive_wrapper<T>, Ts...>
  {
    static constexpr std::int32_t value = Index;
  };

  template <std::int32_t Index, typename T, typename U, typename... Ts>
  struct index_of_optional_impl<Index, T, U, Ts...>
  {
    static constexpr std::int32_t value =
      index_of_optional_impl<Index + 1, T, Ts...>::value;
  };
}

/// Specialization of index_of_optional for container type vector.
template <typename... Ts, typename T>
struct index_of_optional<vector<Ts...>, T>
{
  static constexpr std::int32_t value =
    detail::index_of_optional_impl<0, T, Ts...>::value;
};

/// Specialization of index_of_optional for container type set.
template <typename... Ts, typename T>
struct index_of_optional<set<Ts...>, T>
{
  static constexpr std::int32_t value =
    detail::index_of_optional_impl<0, T, Ts...>::value;
};

/// Specialization of index_of_optional for container type multimap.
/// @remarks
///   If Key exists multiple times then only the first index is returned.
template <typename... Pairs, typename Key>
struct index_of_optional<multimap<Pairs...>, Key>
{
  static constexpr std::int32_t value =
    index_of_optional<transform_t<multimap<Pairs...>, set<>, first>,
                      Key>::value;
};

/// Specialization of index_of_optional for container type map.
template <typename... Pairs, typename Key>
struct index_of_optional<map<Pairs...>, Key>
{
  static constexpr std::int32_t value =
    index_of_optional<transform_t<map<Pairs...>, set<>, first>, Key>::value;
};

/// End of recursion
template <template <typename...> class Container, typename T>
struct in<Container<>, T> : public std::false_type
{
};

template <template <typename...> class Container, typename T, typename Head,
          typename... Ts>
struct in<Container<Head, Ts...>, T>
: public std::conditional_t<std::is_same<T, Head>::value, std::true_type,
                            in<Container<Ts...>, T>>
{
};

template <template <typename...> class Container, typename... Ts,
          typename... Us>
struct intersection<Container<Ts...>, Container<Us...>>
{
  template <typename Destination, typename T>
  using conditional_insert_t =
    std::conditional<in<Container<Us...>, T>::value, insert_t<Destination, T>,
                     Destination>;

  using type = fold_t<Container<Ts...>, Container<>, conditional_insert_t>;
};

/// Sets with just a single type are valid.
template <typename T>
struct is_valid<set<T>> : public std::true_type
{
};

/// Sets are only valid if their typelist Ts is unique.
template <typename T, typename... Ts>
struct is_valid<set<T, Ts...>>
: public std::conditional_t<index_of_optional<set<Ts...>, T>::value == -1 &&
                              is_valid<set<Ts...>>::value,
                            std::true_type, std::false_type>
{
};

/// Any other container is always valid.
template <template <typename...> class Container, typename... Ts>
struct is_valid<Container<Ts...>> : public std::true_type
{
};

template <typename... Ts, typename T>
struct push_front<vector<Ts...>, T>
{
  using type = vector<T, Ts...>;
};

template <typename... Ts, typename T>
struct push_front<set<Ts...>, T>
{
  static_assert(index_of_optional<set<Ts...>, T>::value < 0,
                "Cannot push front type T because it already exists in set.");

  using type = set<T, Ts...>;
};

template <typename... Ts, typename T>
struct push_back<vector<Ts...>, T>
{
  using type = vector<Ts..., T>;
};

template <typename... Ts, typename T>
struct push_back<set<Ts...>, T>
{
  static_assert(index_of_optional<set<Ts...>, T>::value < 0,
                "Cannot push back type T because it already exists in set.");

  using type = set<Ts..., T>;
};

/// Specialization of insert for Container = mpl::vector.
template <typename... Ts, typename T>
struct insert<vector<Ts...>, T>
{
  using type = typename push_back<vector<Ts...>, T>::type;
};

/// Specialization of insert for Container = mpl::set.
template <typename... Ts, typename T>
struct insert<set<Ts...>, T>
{
  using type = std::conditional_t<(index_of_optional<set<Ts...>, T>::value < 0),
                                  set<Ts..., T>, set<Ts...>>;
};

/// Specialization of insert for Container = mpl::multimap.
template <typename... Pairs, typename K, typename V>
struct insert<multimap<Pairs...>, pair<K, V>>
{
  using type = multimap<Pairs..., pair<K, V>>;
};

/// Specialization of insert for Container = mpl::map.
template <typename... Pairs, typename K, typename V>
struct insert<map<Pairs...>, pair<K, V>>
{
  // Only allow insertion of key-value pairs, if the key K is not already
  // present in the map.
  static_assert(index_of_optional<map<Pairs...>, K>::value < 0,
                "Key K already exists in the map.");

  using type = map<Pairs..., pair<K, V>>;
};

/// End recursion when source container is empty.
template <template <typename...> class Source, typename Destination,
          template <typename Container, typename Element> class Transform>
struct fold<Source<>, Destination, Transform>
{
  using type = Destination;
};

/// Recursively transform all source elements into destination.
template <template <typename...> class Source, typename Destination,
          template <typename Container, typename T> class Transform, typename T,
          typename... Ts>
struct fold<Source<T, Ts...>, Destination, Transform>
{
  using type =
    typename fold<Source<Ts...>, typename Transform<Destination, T>::type,
                  Transform>::type;
};

namespace detail
{
  template <typename LeftContainer, typename RightContainer, typename T,
            typename Order,
            template <typename, typename, typename> class Comparator>
  struct sorted_insert_impl;

  /// End of recursion
  template <typename LeftContainer, template <typename...> class RightContainer,
            typename T, typename Order,
            template <typename, typename, typename> class Comparator>
  struct sorted_insert_impl<LeftContainer, RightContainer<>, T, Order,
                            Comparator>
  {
    using type = insert_t<LeftContainer, T>;
  };

  /// Check if T fits between LeftContainer and RightContainer<RHead, RTs...>.
  template <typename LeftContainer, template <typename...> class RightContainer,
            typename RHead, typename... RTs, typename T, typename Order,
            template <typename, typename, typename> class Comparator>
  struct sorted_insert_impl<LeftContainer, RightContainer<RHead, RTs...>, T,
                            Order, Comparator>
  {
    using type = std::conditional_t<
      Comparator<Order, T, RHead>::value,
      fold_t<set<RTs...>, insert_t<insert_t<LeftContainer, T>, RHead>, insert>,
      typename sorted_insert_impl<insert_t<LeftContainer, RHead>,
                                  RightContainer<RTs...>, T, Order,
                                  Comparator>::type>;
  };
}

/// Specialization of sorted_insert for Container = mpl::vector.
template <typename... Ts, typename T, typename Order,
          template <typename, typename, typename> class Comparator>
struct sorted_insert<vector<Ts...>, T, Order, Comparator>
{
  using type = typename detail::sorted_insert_impl<vector<>, vector<Ts...>, T,
                                                   Order, Comparator>::type;
};

/// Specialization of sorted_insert for Container = mpl::set.
template <typename... Ts, typename T, typename Order,
          template <typename, typename, typename> class Comparator>
struct sorted_insert<set<Ts...>, T, Order, Comparator>
{
  using type =
    std::conditional_t<(index_of_optional<set<Ts...>, T>::value < 0),
                       typename detail::sorted_insert_impl<
                         set<>, set<Ts...>, T, Order, Comparator>::type,
                       set<Ts...>>;
};

template <template <typename...> class Container, typename... Ts>
struct concat<Container<Ts...>>
{
  using type = Container<Ts...>;
};

template <template <typename...> class Container, typename... Ts,
          typename... Us, typename... OtherContainers>
struct concat<Container<Ts...>, Container<Us...>, OtherContainers...>
{
  using type =
    typename concat<Container<Ts..., Us...>, OtherContainers...>::type;
};

/// End of recursion.
template <template <typename...> class Source, typename Destination,
          template <typename Element> class Transform>
struct transform<Source<>, Destination, Transform>
{
  using type = Destination;
};

/// Recursively transform all source elements into destination.
template <template <typename...> class Source, typename Destination,
          template <typename T> class Transform, typename T, typename... Ts>
struct transform<Source<T, Ts...>, Destination, Transform>
{
  using type =
    typename transform<Source<Ts...>,
                       insert_t<Destination, typename Transform<T>::type>,
                       Transform>::type;
};

/// finds values within a map, which are associated with a Key.
template <typename... Pairs, typename Key>
struct find<multimap<Pairs...>, Key>
{
private:
  template <typename Container, typename Pair>
  struct helper
  {
    static_assert(is_pair_v<Pair>);

    using type =
      std::conditional_t<std::is_same<typename Pair::first, Key>::value,
                         insert_t<Container, typename Pair::second>, Container>;
  };

public:
  using type = fold_t<multimap<Pairs...>, vector<>, helper>;
};

/// Returns either a matching value type, or no_type if the map does not
/// contain a key Key.
template <typename... Pairs, typename Key>
struct find<map<Pairs...>, Key>
{
  using type = typename find<multimap<Pairs...>, Key>::type::head;
};

/// Implementation of reverse_pair.
template <typename T1, typename T2>
struct reverse_pair<pair<T1, T2>>
{
  using type = pair<T2, T1>;
};

/// Implementation of reverse_map.
template <typename... Pairs>
struct reverse_map<map<Pairs...>>
{
  using type = std::conditional_t<
    (make_set_t<typename Pairs::second...>::size == sizeof...(Pairs)),
    map<reverse_pair_t<Pairs>...>, multimap<reverse_pair_t<Pairs>...>>;
};

/// End of recursion.
template <template <typename...> class Source,
          template <typename Element, typename...> class Predicate,
          typename... Args>
struct select<Source<>, Predicate, Args...>
{
  using type = Source<>;
};

/// Recursively transform all source elements into destination.
template <template <typename...> class Source,
          template <typename Element, typename...> class Predicate,
          typename... Args, typename T, typename... Ts>
struct select<Source<T, Ts...>, Predicate, Args...>
{
  using type = concat_t<
    std::conditional_t<Predicate<T, Args...>::value, Source<T>, Source<>>,
    typename select<Source<Ts...>, Predicate, Args...>::type>;
};

/// Creates a new set of unique types.
template <typename... Ts>
struct make_set
{
  using type = fold_t<vector<Ts...>, set<>, insert>;
};

template <template <typename...> class Container, typename... Ts>
struct to_std_tuple<Container<Ts...>>
{
  using type = std::tuple<Ts...>;
};

template <template <typename...> class Container, typename... Ts>
struct to_vector<Container<Ts...>>
{
  using type = vector<Ts...>;
};

/// Special case for empty source containers.
template <template <typename...> class Container>
struct to_set<Container<>>
{
  using type = set<>;
};

template <template <typename...> class Container, typename T, typename... Ts>
struct to_set<Container<T, Ts...>>
{
  using type = fold_t<Container<T, Ts...>, set<>, insert>;
};

/// End of recursion
template <template <typename...> class Container>
struct for_each<Container<>>
{
  template <typename Visitor, typename... Args>
  inline for_each(Visitor& /*visitor*/, Args&&... /*args*/)
  {
    // NOP.
  }

  template <typename Visitor, typename... Args>
  inline for_each(const Visitor& /*visitor*/, Args&&... /*args*/)
  {
    // NOP.
  }
};

/// Recursively calls visit::operator() for each type T in Ts.
template <template <typename...> class Container, typename T, typename... Ts>
struct for_each<Container<T, Ts...>>
{
  /// Constructor taking a type with an operator()(const T*, Args&&...) for each
  /// type T in Container<Ts...>.
  template <typename Visitor, typename... Args>
  for_each(Visitor& visitor, Args&&... args)
  {
    // The argument is used only for overload resolution.
    visitor(static_cast<const T*>(nullptr), args...);
    for_each<Container<Ts...>> f(visitor, std::forward<Args>(args)...);
  }

  /// Constructor taking a type with an operator()(const T*, Args&&...) const
  /// for each type T in Container<Ts...>.
  template <typename Visitor, typename... Args>
  for_each(const Visitor& visitor, Args&&... args)
  {
    // The argument is used only for overload resolution.
    visitor(static_cast<const T*>(nullptr), args...);
    for_each<Container<Ts...>> f(visitor, std::forward<Args>(args)...);
  }
};

template <typename Container>
struct recursion_helper;

/// End of recursion
template <template <typename...> class Container>
struct recursion_helper<Container<>>
{
  template <typename Visitor>
  inline typename Visitor::return_type operator()(Visitor& visitor) const
  {
    recursion_helper<Container<>> helper;
    return visitor(helper);
  }

  template <typename Visitor>
  inline typename Visitor::return_type operator()(const Visitor& visitor) const
  {
    recursion_helper<Container<>> helper;
    return visitor(helper);
  }
};

/// Recursively calls visit::operator() for each type T in Ts.
template <template <typename...> class Container, typename T, typename... Ts>
struct recursion_helper<Container<T, Ts...>>
{
  /// Constructor taking a type with an operator()(const T*) for each T in Ts.
  template <typename Visitor>
  inline typename Visitor::return_type operator()(Visitor& visitor) const
  {
    // The first argument is used only for overload resolution.
    recursion_helper<Container<Ts...>> helper;
    return visitor(helper, static_cast<const T*>(nullptr));
  }

  /// Constructor taking a type with an operator()(const T*) for each T in Ts.
  template <typename Visitor>
  inline typename Visitor::return_type operator()(const Visitor& visitor) const
  {
    // The first argument is used only for overload resolution.
    recursion_helper<Container<Ts...>> helper;
    return visitor(helper, static_cast<const T*>(nullptr));
  }
};

/// Nothing but an empty type.
template <typename T = void>
struct empty_type
{
};

/// A type used to wrap constant values. This is useful for storing values in
/// MPL containers, that can only handle types.
template <typename T, T Value>
struct value_to_type
{
  using type = T;
  static constexpr T value = Value;
};

/// A helper function that converts any type to a value.
template <typename T>
inline constexpr empty_type<T> type_to_value()
{
  return {};
}

template <typename T, typename TBase>
class default_linear_hierarchy_unit : public T, public TBase
{
};

///
template <typename TList,
          template <typename T, typename TBase> class TUnit =
            default_linear_hierarchy_unit,
          typename TRoot = empty_type<>>
struct gen_linear_hierarchy;

///
template <template <typename...> class Container,
          template <typename, typename> class TUnit, typename TRoot>
struct gen_linear_hierarchy<Container<>, TUnit, TRoot> : public TRoot
{
};

///
template <template <typename...> class Container, typename T, typename... Ts,
          template <typename, typename> class TUnit, typename TRoot>
struct gen_linear_hierarchy<Container<T, Ts...>, TUnit, TRoot>
: public TUnit<T, gen_linear_hierarchy<vector<Ts...>, TUnit, TRoot>>
{
};
}

#endif
