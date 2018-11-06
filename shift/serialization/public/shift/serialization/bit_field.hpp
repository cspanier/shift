#ifndef SHIFT_SERIALIZATION_BITFIELD_HPP
#define SHIFT_SERIALIZATION_BITFIELD_HPP

#include "shift/core/bit_field.hpp"
#include "shift/serialization/types.hpp"
#include "shift/serialization/archive.hpp"

namespace shift::serialization
{
///
template <class InputArchive, typename U>
InputArchive& operator>>(InputArchive& archive, core::bit_field<U>& bitField)
{
  archive >> bitField.data();
  return archive;
}

///
template <class OutputArchive, typename U>
OutputArchive& operator<<(OutputArchive& archive,
                          const core::bit_field<U>& bitField)
{
  archive << bitField.data();
  return archive;
}
}

#endif
