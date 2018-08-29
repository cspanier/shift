#include "shift/serialization/message.h"
#include "shift/serialization/compact/inputarchive.h"
#include "shift/serialization/compact/outputarchive.h"

namespace shift::serialization
{
message::~message() = default;

message_uid_t message::unique_id() const
{
  return _uid;
}

void message::unique_id(message_uid_t uid)
{
  _uid = uid;
}

compact_input_archive<boost::endian::order::big>& operator>>(
  compact_input_archive<boost::endian::order::big>& archive, message& message)
{
  message.read_virtual(archive);
  return archive;
}

compact_output_archive<boost::endian::order::big>& operator<<(
  compact_output_archive<boost::endian::order::big>& archive,
  const message& message)
{
  message.write_virtual(archive);
  return archive;
}

compact_input_archive<boost::endian::order::little>& operator>>(
  compact_input_archive<boost::endian::order::little>& archive,
  message& message)
{
  message.read_virtual(archive);
  return archive;
}

compact_output_archive<boost::endian::order::little>& operator<<(
  compact_output_archive<boost::endian::order::little>& archive,
  const message& message)
{
  message.write_virtual(archive);
  return archive;
}
}
