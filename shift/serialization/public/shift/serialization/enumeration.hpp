#ifndef SHIFT_SERIALIZATION_ENUMERATION_HPP
#define SHIFT_SERIALIZATION_ENUMERATION_HPP

#include <type_traits>
#include "shift/serialization/types.hpp"
#include "shift/serialization/archive.hpp"

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
  archive << core::underlying_type_cast(value);
  return archive;
}
}

#endif
