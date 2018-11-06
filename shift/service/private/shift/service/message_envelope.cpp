#include "shift/service/message_envelope.hpp"
#include <shift/serialization/compact/inputarchive.hpp>
#include <shift/serialization/compact/outputarchive.hpp>

namespace shift::service
{
message_envelope::~message_envelope() = default;

bool message_envelope::begin_read(
  serialization::compact_input_archive<boost::endian::order::big>& archive,
  serialization::message_uid_t& uid)

{
  auto mask = archive.exceptions();
  archive.exceptions(serialization::compact_input_archive<
                     boost::endian::order::big>::stream_t::goodbit);
  archive >> uid;
  if (archive.eof())
    return false;
  archive.exceptions(mask);
  return true;
}

void message_envelope::end_read(
  serialization::compact_input_archive<boost::endian::order::big>& /*archive*/,
  serialization::message_uid_t /*uid*/)
{
  // NOP.
}

bool message_envelope::begin_read(
  serialization::compact_input_archive<boost::endian::order::little>& archive,
  serialization::message_uid_t& uid)

{
  auto mask = archive.exceptions();
  archive.exceptions(serialization::compact_input_archive<
                     boost::endian::order::little>::stream_t::goodbit);
  archive >> uid;
  if (archive.eof())
    return false;
  archive.exceptions(mask);
  return true;
}

void message_envelope::end_read(serialization::compact_input_archive<
                                  boost::endian::order::little>& /*archive*/,
                                serialization::message_uid_t /*uid*/)
{
  // NOP.
}

void message_envelope::begin_write(
  serialization::compact_output_archive<boost::endian::order::big>& archive,
  serialization::message_uid_t uid)
{
  archive << uid;
}

void message_envelope::end_write(
  serialization::compact_output_archive<boost::endian::order::big>& /*archive*/,
  serialization::message_uid_t /*uid*/)
{
  // NOP.
}

void message_envelope::begin_write(
  serialization::compact_output_archive<boost::endian::order::little>& archive,
  serialization::message_uid_t uid)
{
  archive << uid;
}

void message_envelope::end_write(serialization::compact_output_archive<
                                   boost::endian::order::little>& /*archive*/,
                                 serialization::message_uid_t /*uid*/)
{
  // NOP.
}
}
