#ifndef SHIFT_SERIALIZATION2_COMPACT_CHRONO_H
#define SHIFT_SERIALIZATION2_COMPACT_CHRONO_H

#include <chrono>
#include "shift/serialization2/types.h"

namespace shift::serialization2
{
///
template <boost::endian::order Order>
compact_input_archive<Order>& operator>>(
  compact_input_archive<Order>& archive,
  std::chrono::system_clock::time_point& timePoint)
{
  using namespace std::chrono;

  std::uint64_t count;
  archive >> count;
  timePoint = system_clock::time_point(
    duration_cast<system_clock::duration>(nanoseconds(count)));
  return archive;
}

///
template <boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive,
  const std::chrono::system_clock::time_point& timePoint)
{
  using namespace std::chrono;

  std::uint64_t count =
    duration_cast<nanoseconds>(timePoint.time_since_epoch()).count();

  archive << count;
  return archive;
}
}

#endif
