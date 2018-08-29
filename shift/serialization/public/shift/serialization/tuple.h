#ifndef SHIFT_SERIALIZATION_TUPLE_H
#define SHIFT_SERIALIZATION_TUPLE_H

#include <tuple>
#include "shift/serialization/types.h"
#include "shift/serialization/archive.h"

namespace shift::serialization
{
namespace detail
{
  template <class InputArchive, std::size_t N, typename... Us,
            ENABLE_IF(N < sizeof...(Us))>
  void read(InputArchive& archive, std::tuple<Us...>& tuple);

  template <class InputArchive, std::size_t N, typename... Us,
            ENABLE_IF(N >= sizeof...(Us))>
  void read(InputArchive& archive, std::tuple<Us...>& tuple);

  template <class OutputArchive, std::size_t N, typename... Us,
            ENABLE_IF(N < sizeof...(Us))>
  void write(OutputArchive& archive, const std::tuple<Us...>& tuple);

  template <class OutputArchive, std::size_t N, typename... Us,
            ENABLE_IF(N >= sizeof...(Us))>
  void write(OutputArchive& archive, const std::tuple<Us...>& tuple);

  /// Recursively loops through the tuple to read each element sequentially.
  template <class InputArchive, std::size_t N, typename... Us,
            ENABLE_IF_DEF(N < sizeof...(Us))>
  void read(InputArchive& archive, std::tuple<Us...>& tuple)
  {
    archive >> std::get<N>(tuple);
    read<InputArchive, N + 1, Us...>(archive, tuple);
  }

  /// End of recursion.
  template <class InputArchive, std::size_t N, typename... Us,
            ENABLE_IF_DEF(N >= sizeof...(Us))>
  void read(InputArchive& /*archive*/, std::tuple<Us...>& /*tuple*/)
  {
    // NOP.
  }

  /// Recursively loops through the tuple to write each element sequentially.
  template <class OutputArchive, std::size_t N, typename... Us,
            ENABLE_IF_DEF(N < sizeof...(Us))>
  void write(OutputArchive& archive, const std::tuple<Us...>& tuple)
  {
    archive << std::get<N>(tuple);
    write<OutputArchive, N + 1, Us...>(archive, tuple);
  }

  /// End of recursion.
  template <class OutputArchive, std::size_t N, typename... Us,
            ENABLE_IF_DEF(N >= sizeof...(Us))>
  void write(OutputArchive& /*archive*/, const std::tuple<Us...>& /*tuple*/)
  {
    // NOP.
  }
}

///
template <class InputArchive, typename... Us>
InputArchive& operator>>(InputArchive& archive, std::tuple<Us...>& tuple)
{
  archive >> begin_tuple{sizeof...(Us)};
  detail::read<InputArchive, 0, Us...>(archive, tuple);
  archive >> end_tuple{};
  return archive;
}

///
template <class OutputArchive, typename... Us>
OutputArchive& operator<<(OutputArchive& archive,
                          const std::tuple<Us...>& tuple)
{
  archive << begin_tuple{sizeof...(Us)};
  detail::write<OutputArchive, 0, Us...>(archive, tuple);
  archive << end_tuple{};
  return archive;
}
}

#endif
