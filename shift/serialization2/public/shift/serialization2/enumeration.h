#ifndef SHIFT_SERIALIZATION2_ENUMERATION_H
#define SHIFT_SERIALIZATION2_ENUMERATION_H

#include <type_traits>
#include "shift/serialization2/types.h"

namespace shift::serialization2
{
///
template <boost::endian::order Order, typename U,
          ENABLE_IF(std::is_enum<U>::value)>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         U& value)
{
  archive >> *reinterpret_cast<std::underlying_type_t<U>*>(&value);
  return archive;
}

///
template <boost::endian::order Order, typename U,
          ENABLE_IF(std::is_enum<U>::value)>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, U value)
{
  archive << static_cast<std::underlying_type_t<U>>(value);
  return archive;
}
}

#endif
