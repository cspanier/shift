#ifndef SHIFT_SERIALIZATION_ARCHIVE_H
#define SHIFT_SERIALIZATION_ARCHIVE_H

#include <memory>
#include <vector>
#include <streambuf>
#include <shift/core/boost_disable_warnings.h>
#include <boost/iostreams/filtering_stream.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/group_ptr.h>
#include "shift/serialization/types.h"
#include "shift/serialization/message.h"

namespace shift::serialization
{
///
struct begin_read
{
  const char* name = nullptr;
  message_uid_t uid = 0;
};

///
struct end_read
{
};

///
struct begin_write
{
  const char* name = nullptr;
  message_uid_t uid = 0;
};

///
struct end_write
{
};

///
struct begin_field
{
  const char* name = nullptr;
};

///
struct end_field
{
};

///
struct begin_pair
{
};

///
struct end_pair
{
};

///
struct begin_tuple
{
  std::size_t length = 0;
};

///
struct end_tuple
{
};

///
struct begin_array
{
  std::size_t length = 0;
};

///
struct end_array
{
};

///
struct begin_vector
{
  std::size_t length = 0;
};

///
struct end_vector
{
};

///
struct begin_list
{
  std::size_t length = 0;
};

///
struct end_list
{
};

///
struct begin_set
{
  std::size_t length = 0;
};

///
struct end_set
{
};

///
struct begin_map
{
  std::size_t length = 0;
};

///
struct end_map
{
};

///
struct begin_raw_pointer
{
  bool valid;
};

///
struct end_raw_pointer
{
};

///
struct begin_shared_pointer
{
  core::group::key_t key = 0;
};

///
struct end_shared_pointer
{
};

///
template <typename Stream, boost::endian::order Order>
class archive
{
public:
  using stream_t = Stream;

  /// Constructor.
  archive(serialization::protocol_version_t protocol_version)
  : _protocol_version(protocol_version)
  {
    _stream.exceptions(stream_t::failbit | stream_t::eofbit);
  }

  virtual ~archive()
  {
    reset();
  }

  /// Returns the currently set protocol version.
  serialization::protocol_version_t protocol_version()
  {
    return _protocol_version;
  }

  /// Changes the protocol version used to read/write data.
  void protocol_version(serialization::protocol_version_t protocol_version)
  {
    _protocol_version = protocol_version;
  }

  /// Returns the object cache instance.
  std::shared_ptr<core::group> cache()
  {
    return _cache;
  }

  /// Push a new pipeline object to the underlying stream (filter or data
  /// sink/source).
  template <typename Pipeline>
  void push(Pipeline&& pipeline)
  {
    _stream.push(std::forward<Pipeline>(pipeline));
  }

  /// Returns whether end-of-file of the underlying stream has been reached.
  bool eof() const
  {
    return _stream.eof();
  }

  ///
  std::streampos tellg()
  {
    return _stream.tellg();
  }

  ///
  void seekg(std::streampos position)
  {
    _stream.seekg(position);
  }

  ///
  void seekg(std::streamoff offset, std::ios_base::seekdir way)
  {
    _stream.seekg(offset, way);
  }

  /// Resets the underlying stream. This closes all registered filters, sinks,
  /// and sources using their close method.
  void reset()
  {
    return _stream.reset();
  }

  /// Gets the exception mask of the underlying stream.
  std::ios_base::iostate exceptions() const
  {
    return _stream.exceptions();
  }

  /// Sets a new exception mask of the underlying stream.
  void exceptions(std::ios_base::iostate except)
  {
    _stream.clear();
    _stream.exceptions(except);
  }

protected:
  stream_t _stream;
  protocol_version_t _protocol_version;
  std::shared_ptr<core::group> _cache = std::make_shared<core::group>();
};
}

#endif
