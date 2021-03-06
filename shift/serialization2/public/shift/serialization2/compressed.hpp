#ifndef SHIFT_SERIALIZATION2_COMPACT_COMPRESSED_HPP
#define SHIFT_SERIALIZATION2_COMPACT_COMPRESSED_HPP

#include <vector>
#include "shift/serialization2/types.hpp"

namespace shift::serialization2
{
/// A length type which gets compressed in a data stream.
template <typename T>
class compressed
{
public:
  using Type = T;

  compressed() = default;

  inline compressed(T value) : _value(value)
  {
  }

  inline operator Type() const
  {
    return _value;
  }

  inline compressed& operator=(T value)
  {
    _value = value;
    return *this;
  }

private:
  Type _value = 0;
};
}

#endif
