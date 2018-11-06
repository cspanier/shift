#ifndef SHIFT_SERIALIZATION2_TUPLE_HPP
#define SHIFT_SERIALIZATION2_TUPLE_HPP

#include <tuple>
#include "shift/serialization2/types.hpp"

namespace shift::serialization2
{
namespace detail
{
  template <boost::endian::order Order, std::size_t N, typename... Us,
            ENABLE_IF(N < sizeof...(Us))>
  void read(compact_input_archive<Order>& archive, std::tuple<Us...>& tuple);

  template <boost::endian::order Order, std::size_t N, typename... Us,
            ENABLE_IF(N >= sizeof...(Us))>
  void read(compact_input_archive<Order>& archive, std::tuple<Us...>& tuple);

  template <boost::endian::order Order, std::size_t N, typename... Us,
            ENABLE_IF(N < sizeof...(Us))>
  void write(compact_output_archive<Order>& archive,
             const std::tuple<Us...>& tuple);

  template <boost::endian::order Order, std::size_t N, typename... Us,
            ENABLE_IF(N >= sizeof...(Us))>
  void write(compact_output_archive<Order>& archive,
             const std::tuple<Us...>& tuple);

  /// Recursively loops through the tuple to read each element sequentially.
  template <boost::endian::order Order, std::size_t N, typename... Us,
            ENABLE_IF_DEF(N < sizeof...(Us))>
  void read(compact_input_archive<Order>& archive, std::tuple<Us...>& tuple)
  {
    archive >> std::get<N>(tuple);
    read<Order, N + 1, Us...>(archive, tuple);
  }

  /// End of recursion.
  template <boost::endian::order Order, std::size_t N, typename... Us,
            ENABLE_IF_DEF(N >= sizeof...(Us))>
  void read(compact_input_archive<Order>& /*archive*/,
            std::tuple<Us...>& /*tuple*/)
  {
    // NOP.
  }

  /// Recursively loops through the tuple to write each element sequentially.
  template <boost::endian::order Order, std::size_t N, typename... Us,
            ENABLE_IF_DEF(N < sizeof...(Us))>
  void write(compact_output_archive<Order>& archive,
             const std::tuple<Us...>& tuple)
  {
    archive << std::get<N>(tuple);
    write<Order, N + 1, Us...>(archive, tuple);
  }

  /// End of recursion.
  template <boost::endian::order Order, std::size_t N, typename... Us,
            ENABLE_IF_DEF(N >= sizeof...(Us))>
  void write(compact_output_archive<Order>& /*archive*/,
             const std::tuple<Us...>& /*tuple*/)
  {
    // NOP.
  }
}

///
template <boost::endian::order Order, typename... Us>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::tuple<Us...>& tuple)
{
  archive >> begin_tuple{sizeof...(Us)};
  detail::read<Order, 0, Us...>(archive, tuple);
  archive >> end_tuple{};
  return archive;
}

///
template <boost::endian::order Order, typename... Us>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::tuple<Us...>& tuple)
{
  archive << begin_tuple{sizeof...(Us)};
  detail::write<Order, 0, Us...>(archive, tuple);
  archive << end_tuple{};
  return archive;
}
}

#endif
