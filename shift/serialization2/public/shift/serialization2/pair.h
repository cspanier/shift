#ifndef SHIFT_SERIALIZATION2_PAIR_H
#define SHIFT_SERIALIZATION2_PAIR_H

#include <utility>
#include "shift/serialization2/types.h"

namespace shift::serialization2
{
///
template <boost::endian::order Order, typename U, typename V>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::pair<U, V>& pair)
{
  archive >> begin_pair{} >> pair.first >> pair.second >> end_pair{};
  return archive;
}

///
template <boost::endian::order Order, typename U, typename V>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::pair<U, V>& pair)
{
  archive << begin_pair{} << pair.first << pair.second << end_pair{};
  return archive;
}
}

#endif
