#ifndef SHIFT_SERIALIZATION2_ENUMERATION_HPP
#define SHIFT_SERIALIZATION2_ENUMERATION_HPP

#include <type_traits>
#include "shift/serialization2/types.hpp"

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
  archive << core::underlying_type_cast(value);
  return archive;
}
}

#endif
