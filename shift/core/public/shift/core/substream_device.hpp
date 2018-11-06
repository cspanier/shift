#ifndef SHIFT_CORE_SUBSTREAMDEVICE_HPP
#define SHIFT_CORE_SUBSTREAMDEVICE_HPP

#include <algorithm>
#include <iosfwd>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::core
{
template <typename ParentStream>
class substream_device
{
public:
  using char_type = typename ParentStream::char_type;
  using category = boost::iostreams::seekable_device_tag;
  using off_type = typename ParentStream::off_type;
  using pos_type = typename ParentStream::pos_type;

  /// Constructor.
  substream_device(ParentStream& parent_stream, off_type offset, pos_type size)
  : _parent_stream(parent_stream)
  {
    using std::min;

    _parent_stream.seekg(0, std::ios_base::end);
    auto _parent_size = _parent_stream.tellg();
    _parent_stream.seekg(0, std::ios_base::beg);

    _offset = std::min(offset, static_cast<off_type>(_parent_size));
    _size = min(size, _parent_size - _offset);
  }

  ///
  std::streamsize read(char_type* destination, std::streamsize count)
  {
    using std::min;

    count = min(count, static_cast<std::streamsize>(_size - _position));
    if (count <= 0)
      return -1;  // EOF

    _parent_stream.seekg(_offset + _position, std::ios_base::beg);
    _parent_stream.read(destination, count);
    _position += count;
    return count;
  }

  ///
  std::streamsize write(const char_type* source, std::streamsize count)
  {
    using std::min;

    count = min(count, static_cast<std::streamsize>(_size - _position));
    if (count <= 0)
      return 0;  // EOF

    _parent_stream.seekp(_offset + _position, std::ios_base::beg);
    _parent_stream.write(source, count);
    _position += count;
    return count;
  }

  ///
  boost::iostreams::stream_offset seek(boost::iostreams::stream_offset offset,
                                       std::ios_base::seekdir way)
  {
    using namespace std;

    boost::iostreams::stream_offset new_position;
    if (way == ios_base::beg)
      new_position = offset;
    else if (way == ios_base::cur)
      new_position = static_cast<off_type>(_position) + offset;
    else if (way == ios_base::end)
    {
      new_position =
        static_cast<pos_type>(static_cast<off_type>(_size) + offset - 1);
    }
    else
      throw ios_base::failure("bad seek direction");

    if (new_position < 0 || new_position >= _size)
      throw ios_base::failure("bad seek offset");

    _position = new_position;
    return _position;
  }

  ///
  ParentStream& parent_stream()
  {
    return _parent_stream;
  }

private:
  ParentStream& _parent_stream;
  off_type _offset;
  pos_type _size;
  pos_type _position = 0;
};
}

#endif
