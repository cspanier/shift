#ifndef SHIFT_SERIALIZATION_VARIANT_H
#define SHIFT_SERIALIZATION_VARIANT_H

#include <variant>
#include "shift/serialization/types.h"
#include "shift/serialization/archive.h"

namespace shift::serialization::detail
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
  template <class InputArchive, std::size_t N>
  static bool read(InputArchive& archive, std::variant<Us...>& variant,
                   std::uint8_t index, ENABLE_IF(N < sizeof...(Us)))
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
      return variant_helper<Us...>::template read<InputArchive, N + 1>(
        archive, variant, index);
    }
  }

  /// End of recursion.
  template <class InputArchive, std::size_t N>
  [[noreturn]] static bool read(InputArchive& /*archive*/,
                                std::variant<Us...>& /*variant*/,
                                std::uint8_t /*index*/,
                                ENABLE_IF(N >= sizeof...(Us)))
  {
    BOOST_THROW_EXCEPTION(
      SerializationError() << SerializationErrorInfo(
        "Detected invalid type key in variant deserializing routine."));
  }

  /// Recursively loops through the variant to find the element to write.
  template <class OutputArchive, std::size_t N>
  static void write(OutputArchive& archive, const std::variant<Us...>& variant,
                    std::uint8_t index, ENABLE_IF(N < sizeof...(Us)))
  {
    if (N == index)
      archive << std::get<typename get_type<N, Us...>::type>(variant);
    else
    {
      variant_helper<Us...>::template write<OutputArchive, N + 1>(
        archive, variant, index);
    }
  }

  /// End of recursion.
  template <class OutputArchive, std::size_t N>
  static void write(OutputArchive& /*archive*/,
                    const std::variant<Us...>& /*variant*/,
                    std::uint8_t /*index*/, ENABLE_IF(N >= sizeof...(Us)))
  {
    BOOST_ASSERT(false);
  }
};
}

namespace shift::serialization
{
/// A specialization for types std::variant<Us...>.
template <class InputArchive, typename... Us>
InputArchive& operator>>(InputArchive& archive, std::variant<Us...>& variant)
{
  static_assert(sizeof...(Us) <= std::numeric_limits<std::uint8_t>::max());
  std::uint8_t index;
  archive >> index;
  detail::variant_helper<Us...>::template read<InputArchive, 0>(archive,
                                                                variant, index);
  return archive;
}

/// A specialization for types std::variant<Us...>.
template <class OutputArchive, typename... Us>
OutputArchive& operator<<(OutputArchive& archive,
                          const std::variant<Us...>& variant)
{
  static_assert(sizeof...(Us) <= std::numeric_limits<std::uint8_t>::max());
  auto index = static_cast<std::uint8_t>(variant.index());
  archive << index;
  detail::variant_helper<Us...>::template write<OutputArchive, 0>(
    archive, variant, index);
  return archive;
}
}

#endif
