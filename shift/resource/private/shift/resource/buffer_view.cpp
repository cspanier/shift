#include "shift/resource/buffer_view.h"
#include <shift/serialization2/all.h>
#include <shift/crypto/sha256.h>

namespace shift::resource
{
void buffer_view::hash(crypto::sha256& context) const
{
  context << buffer.id() << offset << size;
}

serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, buffer_view& value)
{
  archive >> value.buffer >> value.offset >> value.size;
  return archive;
}

serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive, const buffer_view& value)
{
  archive << value.buffer << value.offset << value.size;
  return archive;
}
}