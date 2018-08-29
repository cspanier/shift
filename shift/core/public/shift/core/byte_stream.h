#ifndef SHIFT_CORE_BYTESTREAM_H
#define SHIFT_CORE_BYTESTREAM_H

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <string>
#include <type_traits>
#include "shift/core/types.h"

namespace shift::core
{
///
template <typename Buffer>
class input_byte_stream
{
public:
  using Buffer = Buffer;

  /// Default constructor.
  input_byte_stream() = default;

  /// Constructor.
  /// @param size
  ///   Size of the internal buffer to preallicate.
  input_byte_stream(std::size_t size)
  {
    if (size)
      _buffer.reserve(size);
  }

  /// Move constructor.
  input_byte_stream(Buffer&& buffer) : _buffer(std::move(buffer))
  {
  }

  /// Copy Constructor.
  input_byte_stream(const input_byte_stream& other) = default;

  /// Swap two buffers with each other.
  void swap(input_byte_stream& other)
  {
    std::swap(_buffer, other._buffer);
    std::swap(_offset, other._offset);
  }

  /// Resets the stream to default state.
  void clear()
  {
    input_byte_stream<Buffer> temp;
    temp.swap(*this);
  }

  /// Returns the internal buffer.
  Buffer& buffer()
  {
    return _buffer;
  }

  /// Returns the internal buffer.
  const Buffer& buffer() const
  {
    return _buffer;
  }

  /// Returns the number of bytes currently stored in the stream.
  std::size_t size() const
  {
    return _buffer.size();
  }

  /// Resizes the stream to a fixed length of new_size. If the old size is
  /// smaller than the new one, any additional bytes are left uninitialized.
  /// If the old size was larger than the new one, trailing bytes are cut off.
  void resize(std::size_t new_size)
  {
    _buffer.resize(new_size);
    if (_offset > new_size)
      _offset = new_size;
  }

  /// Returns the size of the internal buffer.
  std::size_t capacity() const
  {
    return _buffer.capacity();
  }

  /// An overload of the operator for strings.
  input_byte_stream& operator&(std::string& string)
  {
    std::uint32_t length;
    *this >> length;
    if (length > _buffer.size() - _offset)
    {
      // Tried to read a string that is larget than the number of available
      // bytes.
      length = static_cast<decltype(length)>(_buffer.size() - _offset);
    }
    string = std::string((const char*)_buffer.data() + _offset, length);
    _offset += length;
    return *this;
  }

  /// Read arbitrary data from the stream.
  /// @param buffer
  ///   The memory location to write the data to.
  /// @param bytes_to_read
  ///   Number of bytes to read.
  /// @return
  ///   Number of bytes actually read.
  std::size_t read(unsigned char* buffer, std::size_t bytes_to_read)
  {
    std::size_t available = _buffer.size() - _offset;
    if (bytes_to_read < available)
      bytes_to_read = available;
    if (bytes_to_read > 0)
    {
      memcpy(buffer, _buffer.data() + _offset, bytes_to_read);
      _offset += bytes_to_read;
    }
    return bytes_to_read;
  }

private:
  Buffer _buffer;
  std::size_t _offset = 0;
};

///
template <typename Buffer, typename T,
          ENABLE_IF(std::is_trivially_copyable<T>::value)>
input_byte_stream<Buffer>& operator&(input_byte_stream<Buffer>& stream, T& data)
{
  stream.read(reinterpret_cast<unsigned char*>(&data), sizeof(T));
  return stream;
}

///
template <typename Buffer>
class output_byte_stream
{
public:
  using Buffer = Buffer;

  /// Default constructor.
  output_byte_stream() = default;

  /// Constructor.
  /// @param size
  ///   Size of the internal buffer to preallicate.
  output_byte_stream(std::size_t size)
  {
    if (size)
      _buffer.reserve(size);
  }

  /// Move constructor.
  output_byte_stream(Buffer&& buffer) : _buffer(std::move(buffer))
  {
  }

  /// Copy Constructor.
  output_byte_stream(const output_byte_stream& other) = default;

  /// Swap two buffers with each other.
  void swap(output_byte_stream& other)
  {
    std::swap(_buffer, other._buffer);
    std::swap(_offset, other._offset);
  }

  /// Resets the stream to default state.
  void clear()
  {
    output_byte_stream<Buffer> temp;
    temp.swap(*this);
  }

  /// Returns the internal buffer.
  Buffer& buffer()
  {
    return _buffer;
  }

  /// Returns the internal buffer.
  const Buffer& buffer() const
  {
    return _buffer;
  }

  /// Returns the number of bytes currently stored in the stream.
  std::size_t size() const
  {
    return _buffer.size();
  }

  /// Resizes the stream to a fixed length of new_size. If the old size is
  /// smaller than the new one, any additional bytes are left uninitialized.
  /// If the old size was larger than the new one, trailing bytes are cut off.
  void resize(std::size_t new_size)
  {
    _buffer.resize(new_size);
    if (_offset > new_size)
      _offset = new_size;
  }

  /// Returns the size of the internal buffer.
  std::size_t capacity() const
  {
    return _buffer.capacity();
  }

  /// An overload of the operator for strings.
  output_byte_stream& operator&(const std::string& string)
  {
    std::uint32_t length;
    if (string.size() > std::numeric_limits<decltype(length)>::max())
    {
      // Cannot store strings larger than 2^32 characters.
      length = std::numeric_limits<std::uint32_t>::max();
    }
    else
      length = static_cast<decltype(length)>(string.size());
    *this << length;
    write(reinterpret_cast<const unsigned char*>(string.data()), length);
    return *this;
  }

  /// Write arbitrary data to the stream.
  /// @param buffer
  ///   The memory location to read the data from.
  /// @param bytes_to_write
  ///   Number of bytes to write.
  void write(const unsigned char* buffer, std::size_t bytes_to_write)
  {
    std::size_t free = _buffer.size() - _offset;
    if (bytes_to_write > free)
      _buffer.resize(_offset + bytes_to_write);
    memcpy(_buffer.data() + _offset, buffer, bytes_to_write);
    _offset += bytes_to_write;
  }

private:
  Buffer _buffer;
  std::size_t _offset = 0;
};

///
template <typename Buffer, typename T,
          ENABLE_IF(std::is_trivially_copyable<T>::value)>
output_byte_stream<Buffer>& operator&(output_byte_stream<Buffer>& stream,
                                      const T& data)
{
  stream.write(reinterpret_cast<const unsigned char*>(&data), sizeof(T));
  return stream;
}
}

#endif
