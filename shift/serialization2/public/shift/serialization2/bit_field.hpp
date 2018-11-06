#ifndef SHIFT_SERIALIZATION2_BITFIELD_HPP
#define SHIFT_SERIALIZATION2_BITFIELD_HPP

#include "shift/core/bit_field.hpp"
#include "shift/serialization2/types.hpp"

namespace shift::serialization2
{
///
template <boost::endian::order Order, typename U>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         core::bit_field<U>& bitField)
{
  archive >> bitField.data();
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const core::bit_field<U>& bitField)
{
  archive << bitField.data();
  return archive;
}
}

#endif
