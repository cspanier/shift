#ifndef SHIFT_SERIALIZATION_BITFIELD_H
#define SHIFT_SERIALIZATION_BITFIELD_H

#include "shift/core/bit_field.h"
#include "shift/serialization/types.h"
#include "shift/serialization/archive.h"

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
