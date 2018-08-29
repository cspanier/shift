#ifndef SHIFT_SERIALIZATION_ENUMERATION_H
#define SHIFT_SERIALIZATION_ENUMERATION_H

#include <type_traits>
#include "shift/serialization/types.h"
#include "shift/serialization/archive.h"

namespace shift::serialization
{
///
template <class InputArchive, typename U, ENABLE_IF(std::is_enum<U>::value)>
InputArchive& operator>>(InputArchive& archive, U& value)
{
  archive >> *reinterpret_cast<std::underlying_type_t<U>*>(&value);
  return archive;
}

///
template <class OutputArchive, typename U, ENABLE_IF(std::is_enum<U>::value)>
OutputArchive& operator<<(OutputArchive& archive, U value)
{
  archive << static_cast<std::underlying_type_t<U>>(value);
  return archive;
}
}

#endif
