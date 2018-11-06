#ifndef SHIFT_SERIALIZATION2_SET_HPP
#define SHIFT_SERIALIZATION2_SET_HPP

#include <set>
#include "shift/serialization2/types.hpp"

namespace shift::serialization2
{
///
template <boost::endian::order Order, typename U>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::set<U>& set)
{
  set.clear();
  begin_set begin;
  archive >> begin;
  for (std::size_t i = 0; i != begin.length; ++i)
  {
    U element;
    archive >> element;
    if (!set.insert(element).second)
    {
      BOOST_THROW_EXCEPTION(
        serialization_error() << serialization_error_info(
          "Detected duplicate value in set deserialization routine."));
    }
  }
  archive >> end_set{};
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::set<U>& set)
{
  archive << begin_set{set.size()};
  for (const auto& element : set)
    archive << element;
  archive << end_set{};
  return archive;
}
}

#endif
