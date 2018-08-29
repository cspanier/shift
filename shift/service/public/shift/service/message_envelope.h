#ifndef SHIFT_SERVICE_MESSAGEENVELOPE_H
#define SHIFT_SERVICE_MESSAGEENVELOPE_H

#include <shift/serialization/types.h>
#include <shift/serialization/message.h>

namespace shift::service
{
/// The default message envelope, which may be overwritten and customized.
class message_envelope
{
public:
  message_envelope() = default;
  message_envelope(const message_envelope&) = default;
  message_envelope(message_envelope&&) = default;
  virtual ~message_envelope();
  message_envelope& operator=(const message_envelope&) = default;
  message_envelope& operator=(message_envelope&&) = default;

  /// Read the envelope header.
  virtual bool begin_read(shift::serialization::compact_input_archive<
                            boost::endian::order::big>& archive,
                          shift::serialization::message_uid_t& uid);

  /// Read the envelope footer.
  virtual void end_read(shift::serialization::compact_input_archive<
                          boost::endian::order::big>& archive,
                        shift::serialization::message_uid_t uid);

  /// Read the envelope header.
  virtual bool begin_read(shift::serialization::compact_input_archive<
                            boost::endian::order::little>& archive,
                          shift::serialization::message_uid_t& uid);

  /// Read the envelope footer.
  virtual void end_read(shift::serialization::compact_input_archive<
                          boost::endian::order::little>& archive,
                        shift::serialization::message_uid_t uid);

  /// Write the envelope header.
  virtual void begin_write(shift::serialization::compact_output_archive<
                             boost::endian::order::big>& archive,
                           shift::serialization::message_uid_t uid);

  /// Write the envelope footer.
  virtual void end_write(shift::serialization::compact_output_archive<
                           boost::endian::order::big>& archive,
                         shift::serialization::message_uid_t uid);

  /// Write the envelope header.
  virtual void begin_write(shift::serialization::compact_output_archive<
                             boost::endian::order::little>& archive,
                           shift::serialization::message_uid_t uid);

  /// Write the envelope footer.
  virtual void end_write(shift::serialization::compact_output_archive<
                           boost::endian::order::little>& archive,
                         shift::serialization::message_uid_t uid);
};
}

#endif
