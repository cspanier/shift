#ifndef SHIFT_SERIALIZATION_MESSAGE_HPP
#define SHIFT_SERIALIZATION_MESSAGE_HPP

#include <cstdint>
#include <utility>
#include <memory>
#include <array>
#include <vector>
#include "shift/core/group_ptr.hpp"
#include "shift/serialization/types.hpp"

namespace shift::serialization
{
/// Base class for all messages which implement a custom (de-)serialization
/// function.
class message
{
public:
  /// Destructor.
  virtual ~message();

  /// Returns the unique identifier of this message.
  message_uid_t unique_id() const;

  /// Overwrites the message unique identifier.
  /// @remarks
  ///   Use with caution. Changing the uid to some arbitrary value may cause
  ///   havok in message (de-)serialization routines.
  void unique_id(message_uid_t uid);

  ///
  virtual void read_virtual(
    ::shift::serialization::compact_input_archive<boost::endian::order::big>&
      archive) = 0;

  ///
  virtual void write_virtual(
    ::shift::serialization::compact_output_archive<boost::endian::order::big>&
      archive) const = 0;

  ///
  virtual void read_virtual(
    ::shift::serialization::compact_input_archive<boost::endian::order::little>&
      archive) = 0;

  ///
  virtual void write_virtual(::shift::serialization::compact_output_archive<
                             boost::endian::order::little>& archive) const = 0;

protected:
  message_uid_t _uid = 0;
};

///
compact_input_archive<boost::endian::order::big>& operator>>(
  compact_input_archive<boost::endian::order::big>& archive, message& message);

///
compact_output_archive<boost::endian::order::big>& operator<<(
  compact_output_archive<boost::endian::order::big>& archive,
  const message& message);

///
compact_input_archive<boost::endian::order::little>& operator>>(
  compact_input_archive<boost::endian::order::little>& archive,
  message& message);

///
compact_output_archive<boost::endian::order::little>& operator<<(
  compact_output_archive<boost::endian::order::little>& archive,
  const message& message);
}

#endif
