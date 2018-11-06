#ifndef SHIFT_SERIALIZATION2_VARIANT_HPP
#define SHIFT_SERIALIZATION2_VARIANT_HPP

#include <variant>
#include "shift/serialization2/types.hpp"

namespace shift::serialization2::detail
{
template <std::size_t N, typename Head, typename... Ts>
struct get_type
{
  using type = typename get_type<N - 1, Ts...>::type;
};

template <typename Head, typename... Ts>
struct get_type<0, Head, Ts...>
{
  using type = Head;
};

template <typename... Us>
struct variant_helper
{
  /// Recursively loops through the variant to find the element to read.
  template <boost::endian::order Order, std::size_t N>
  static bool read(compact_input_archive<Order>& archive,
                   std::variant<Us...>& variant, std::uint8_t index,
                   ENABLE_IF(N < sizeof...(Us)))
  {
    if (N == index)
    {
      typename get_type<N, Us...>::type value;
      archive >> value;
      variant = std::move(value);
      return true;
    }
    else
    {
      return variant_helper<Us...>::template read<Order, N + 1>(archive,
                                                                variant, index);
    }
  }

  /// End of recursion.
  template <boost::endian::order Order, std::size_t N>
  [[noreturn]] static bool read(compact_input_archive<Order>& /*archive*/,
                                std::variant<Us...>& /*variant*/,
                                std::uint8_t /*index*/,
                                ENABLE_IF(N >= sizeof...(Us)))
  {
    BOOST_THROW_EXCEPTION(
      serialization_error() << serialization_error_info(
        "Detected invalid type key in variant deserializing routine."));
  }

  /// Recursively loops through the variant to find the element to write.
  template <boost::endian::order Order, std::size_t N>
  static void write(compact_output_archive<Order>& archive,
                    const std::variant<Us...>& variant, std::uint8_t index,
                    ENABLE_IF(N < sizeof...(Us)))
  {
    if (N == index)
      archive << std::get<typename get_type<N, Us...>::type>(variant);
    else
    {
      variant_helper<Us...>::template write<Order, N + 1>(archive, variant,
                                                          index);
    }
  }

  /// End of recursion.
  template <boost::endian::order Order, std::size_t N>
  static void write(compact_output_archive<Order>& /*archive*/,
                    const std::variant<Us...>& /*variant*/,
                    std::uint8_t /*index*/, ENABLE_IF(N >= sizeof...(Us)))
  {
    BOOST_ASSERT(false);
  }
};
}

namespace shift::serialization2
{
/// A specialization for types std::variant<Us...>.
template <boost::endian::order Order, typename... Us>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::variant<Us...>& variant)
{
  static_assert(sizeof...(Us) <= std::numeric_limits<std::uint8_t>::max());
  std::uint8_t index;
  archive >> index;
  detail::variant_helper<Us...>::template read<Order, 0>(archive, variant,
                                                         index);
  return archive;
}

/// A specialization for types std::variant<Us...>.
template <boost::endian::order Order, typename... Us>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::variant<Us...>& variant)
{
  static_assert(sizeof...(Us) <= std::numeric_limits<std::uint8_t>::max());
  auto index = static_cast<std::uint8_t>(variant.index());
  archive << index;
  detail::variant_helper<Us...>::template write<Order, 0>(archive, variant,
                                                          index);
  return archive;
}
}

#endif
