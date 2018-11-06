#ifndef SHIFT_RESOURCE_BUFFER_VIEW_HPP
#define SHIFT_RESOURCE_BUFFER_VIEW_HPP

#include <vector>
#include "shift/resource/resource_ptr.hpp"
#include "shift/resource/buffer.hpp"

namespace shift::resource
{
/// A view into a buffer generally representing a subset of the buffer.
class buffer_view
{
public:
  /// Equality operator.
  friend bool operator==(const buffer_view& lhs, const buffer_view& rhs)
  {
    return lhs.buffer == rhs.buffer && lhs.offset == rhs.offset &&
           lhs.size == rhs.size;
  }

  /// Inequality operator.
  friend bool operator!=(const buffer_view& lhs, const buffer_view& rhs)
  {
    return !(lhs == rhs);
  }

  ///
  void hash(crypto::sha256& context) const;

  ///
  friend serialization2::compact_input_archive<>& operator>>(
    serialization2::compact_input_archive<>& archive, buffer_view& value);

  ///
  friend serialization2::compact_output_archive<>& operator<<(
    serialization2::compact_output_archive<>& archive,
    const buffer_view& value);

  /// The referenced buffer.
  resource_ptr<resource::buffer> buffer;
  /// The offset into the buffer in bytes.
  std::uint32_t offset = 0;
  /// The length of the bufferView in bytes.
  std::uint32_t size = 0;
};
}

#endif
