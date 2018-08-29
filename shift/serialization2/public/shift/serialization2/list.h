#ifndef SHIFT_SERIALIZATION2_LIST_H
#define SHIFT_SERIALIZATION2_LIST_H

#include <list>
#include "shift/serialization2/types.h"

namespace shift::serialization2
{
///
template <boost::endian::order Order, typename U>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::list<U>& list)
{
  begin_list begin;
  archive >> begin;
  list.resize(begin.length);
  for (auto& element : list)
    archive >> element;
  archive.end_list();
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::list<U>& list)
{
  archive << begin_list{list.size()};
  for (const auto& element : list)
    archive << element;
  archive << end_list{};
  return archive;
}
}

#endif
