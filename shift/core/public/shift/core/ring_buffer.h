#ifndef SHIFT_CORE_RINGBUFFER_H
#define SHIFT_CORE_RINGBUFFER_H

#include <utility>
#include <algorithm>
#include <cstring>
#include <shift/core/boost_disable_warnings.h>
#include <boost/call_traits.hpp>
#include <shift/core/boost_restore_warnings.h>

namespace shift::core
{
/// A ring buffer using a single linear memory buffer to store elements of
/// various type.
/// @remarks
///   The buffer is organized into up to two regions of memory which are
///   marked using the begin and end member pointers. Use of these is as
///   follows (b1 = _begin1, e1 = _end1, b2 = _begin2, e2 = _end2):
///   Case 1) Buffer is empty:
///   | | | | | | | | | | |
///    ^b1=e1=e2           ^b2
///   Case 2) Buffer is full:
///   |a|b|c|d|e|f|g|h|i|j|
///    ^b1=e2              ^e1=b2
///   Case 3) Data is split into two blocks:
///   |e|f|g| | | |a|b|c|d|
///    ^b2   ^e2   ^b1     ^e1
///   Case 4) Free space is split into two blocks:
///   | | |a|b|c|d|e|f| | |
///    ^e2 ^b1         ^e1 ^b2
template <typename T>
class ring_buffer
{
public:
  using buffer_range_t = std::pair<T*, std::size_t>;
  using const_buffer_range_t = std::pair<const T*, std::size_t>;

  /// Constructor.
  ring_buffer(std::size_t capacity = 4096);

  /// Destructor.
  ~ring_buffer();

  /// Returns the size in bytes of the data currently being stored.
  std::size_t size() const;

  /// Returns the size in bytes of the internal memory buffer.
  std::size_t capacity() const;

  /// Attempts to write a block of data to the ring buffer. If there is not
  /// enough free space the method will write as much as possible.
  /// @param buffer
  ///   A pointer to the memory to be written. If this is null then no data
  ///   is actually written (see ring_buffer::produce).
  /// @param size
  ///   The number of elements in the data block to write to the ring buffer.
  /// @param allow_partial
  ///   When set to true attampts to write more elements than free space is
  ///   available will instruct the method to write as many elements as
  ///   possible to the ring buffer. Otherwise these calls will return without
  ///   writing any data.
  /// @return
  ///   The number of elements that have been written to the ring buffer.
  std::size_t write(const T* buffer, std::size_t size,
                    bool allow_partial = false);

  /// Templated version of ring_buffer::write.
  template <typename U>
  inline std::size_t write(typename boost::call_traits<U>::param_type value,
                           bool allow_partial = false);

  /// Attempts to read a number of elements from the ring buffer.
  /// @param buffer
  ///   The memory location to write the elements to. If this parameter is
  ///   null the method will not copy any elements (see ring_buffer::consume).
  ///   If this parameter is not null it must be large enough to store at
  ///   least size elements.
  /// @param size
  ///   The number of elements to read from the ring buffer.
  /// @param allow_partial
  ///   When set to true attempts to read more elements than are available
  ///   instruct the method to read all available data. Otherwise these
  ///   calls will return without reading any data.
  /// @return
  ///   The number of elements that have been read from the ring buffer.
  std::size_t read(T* buffer, std::size_t size, bool allow_partial = false);

  /// Templated version of ring_buffer::read.
  template <typename U>
  inline std::size_t read(typename boost::call_traits<U>::reference value,
                          bool allow_partial = false);

  /// Produces data without modifying the internal buffer. This is useful
  /// after directly writing to the buffer using spare_array*.
  /// @param size
  ///   The number of elements to produce.
  /// @param allow_partial
  ///   If there is not enough space in the ring buffer and this parameter is
  ///   set to true then this call will write as many elements until the ring
  ///   buffer is full. If this parameter is false attempts to write too many
  ///   elements will be ignored.
  /// @return
  ///   The number of elements successfully produced.
  std::size_t produce(std::size_t size, bool allow_partial = false);

  /// Consumes data without actually reading from the buffer. This is useful
  /// after directly reading from the buffer using data_array*.
  /// @param size
  ///   The number of elements to consume from the ring buffer.
  /// @param allow_partial
  ///   When set to true attempts to read more elements than are available
  ///   will be performed partially. Otherwise these attempts will result in
  ///   no elements being read from the ring buffer.
  /// @return
  ///   The number of elements successfully consumed.
  std::size_t consume(std::size_t size, bool allow_partial = false);

  /// Returns a description of the memory region containing the first block
  /// of data. If there is none the size of the region will be zero and the
  /// pointer will be unspecified.
  buffer_range_t data_array1();

  /// @see ring_buffer::data_array1.
  const_buffer_range_t data_array1() const;

  /// Returns a description of the memory region containing the second block
  /// of data. If there is none the size of the region will be zero and the
  /// pointer will be unspecified.
  buffer_range_t data_array2();

  /// @see ring_buffer::data_array2.
  const_buffer_range_t data_array2() const;

  /// Returns a description of the free memory region behind the first block
  /// of data. If there is none the size of the region will be zero and the
  /// pointer will be unspecified.
  buffer_range_t spare_array1();

  /// @see ring_buffer::spare_array1.
  const_buffer_range_t spare_array1() const;

  /// Returns a description of the free memory region behind the second block
  /// of data. If there is none the size of the region will be zero and the
  /// pointer will be unspecified.
  buffer_range_t spare_array2();

  /// @see ring_buffer::spare_array2.
  const_buffer_range_t spare_array2() const;

private:
  T* _data = nullptr;
  std::size_t _capacity;
  std::size_t _size = 0;
  T* _begin1 = nullptr;
  T* _end1 = nullptr;
  T* _begin2 = nullptr;
  T* _end2 = nullptr;
};

template <typename T>
ring_buffer<T>::ring_buffer(std::size_t capacity) : _capacity(capacity)
{
  _data = new T[_capacity];
  _begin1 = &_data[0];
  _end1 = &_data[0];
  _begin2 = &_data[_capacity];
  _end2 = &_data[0];
}

template <typename T>
ring_buffer<T>::~ring_buffer()
{
  delete _data;
}

template <typename T>
std::size_t ring_buffer<T>::size() const
{
  return _size;
}

template <typename T>
std::size_t ring_buffer<T>::capacity() const
{
  return _capacity;
}

template <typename T>
std::size_t ring_buffer<T>::write(const T* buffer, std::size_t size,
                                  bool allow_partial)
{
  std::size_t free = _capacity - _size;
  if (size == 0 || free == 0)
    return 0;
  if (size > free)
  {
    if (!allow_partial)
      return 0;
    size = free;
  }
  if (_begin2 > _end1)
  {
    // There is free space behind the first data block.
    std::size_t size_block1 = _begin2 - _end1;
    if (size_block1 > size)
    {
      if (buffer)
        memcpy(_end1, buffer, size);
      _end1 += size;
      _size += size;
    }
    else
    {
      std::size_t size_block2 = size - size_block1;
      if (buffer)
        memcpy(_end1, buffer, size_block1);
      _end1 = _begin2;
      // There is not enough space behind data block one, so start the second
      // one.
      _begin2 = _data;
      if (buffer)
        memcpy(_begin2, buffer + size_block1, size_block2);
      _end2 = _begin2 + size_block2;
      _size += size;
    }
  }
  else
  {
    if (buffer)
      memcpy(_end2, buffer, size);
    _end2 += size;
    _size += size;
  }
  return size;
}

template <typename T>
template <typename U>
inline std::size_t ring_buffer<T>::write(
  typename boost::call_traits<U>::param_type value, bool allow_partial)
{
  return write(reinterpret_cast<const T*>(&value), sizeof(U) / sizeof(T),
               allow_partial);
}

template <typename T>
std::size_t ring_buffer<T>::read(T* buffer, std::size_t size,
                                 bool allow_partial)
{
  if (size == 0 || _size == 0)
    return 0;
  if (size > _size)
  {
    if (!allow_partial)
      return 0;
    size = _size;
  }
  std::size_t size_block1 = _end1 - _begin1;
  if (size_block1 > size)
  {
    if (buffer)
      memcpy(buffer, _begin1, size);
    _begin1 += size;
    _size -= size;
  }
  else
  {
    if (buffer)
    {
      memcpy(buffer, _begin1, size_block1);
      if (size_block1 < size)
        memcpy(buffer + size_block1, _data, size - size_block1);
    }

    if (_size > size)
    {
      _begin1 = _begin2 + (size - size_block1);
      _end1 = _end2;
      _size -= size;
    }
    else
    {
      // Ringbuffer is empty now, so completely reset it.
      _begin1 = _data;
      _end1 = _data;
      _size = 0;
    }
    _begin2 = _data + _capacity;
    _end2 = _begin2;
  }
  return size;
}

template <typename T>
template <typename U>
inline std::size_t ring_buffer<T>::read(
  typename boost::call_traits<U>::reference value, bool allow_partial)
{
  return read(reinterpret_cast<T*>(&value), sizeof(U) / sizeof(T),
              allow_partial);
}

template <typename T>
std::size_t ring_buffer<T>::produce(std::size_t size, bool allow_partial)
{
  return write(nullptr, size, allow_partial);
}

template <typename T>
std::size_t ring_buffer<T>::consume(std::size_t size, bool allow_partial)
{
  return read(nullptr, size, allow_partial);
}

template <typename T>
typename ring_buffer<T>::buffer_range_t ring_buffer<T>::data_array1()
{
  return buffer_range_t(_begin1, _end1 - _begin1);
}

template <typename T>
typename ring_buffer<T>::const_buffer_range_t ring_buffer<T>::data_array1()
  const
{
  return const_buffer_range_t(_begin1, _end1 - _begin1);
}

template <typename T>
typename ring_buffer<T>::buffer_range_t ring_buffer<T>::data_array2()
{
  return buffer_range_t(_begin2, std::max<std::ptrdiff_t>(_end2 - _begin2, 0));
}

template <typename T>
typename ring_buffer<T>::const_buffer_range_t ring_buffer<T>::data_array2()
  const
{
  return const_buffer_range_t(_begin2,
                              std::max<std::ptrdiff_t>(_end2 - _begin2, 0));
}

template <typename T>
typename ring_buffer<T>::buffer_range_t ring_buffer<T>::spare_array1()
{
  return buffer_range_t(_end1, std::max<std::ptrdiff_t>(_begin2 - _end1, 0));
}

template <typename T>
typename ring_buffer<T>::const_buffer_range_t ring_buffer<T>::spare_array1()
  const
{
  return const_buffer_range_t(_end1,
                              std::max<std::ptrdiff_t>(_begin2 - _end1, 0));
}

template <typename T>
typename ring_buffer<T>::buffer_range_t ring_buffer<T>::spare_array2()
{
  return buffer_range_t(_end2, std::max<std::ptrdiff_t>(_begin1 - _end2, 0));
}

template <typename T>
typename ring_buffer<T>::const_buffer_range_t ring_buffer<T>::spare_array2()
  const
{
  return const_buffer_range_t(_end2,
                              std::max<std::ptrdiff_t>(_begin1 - _end2, 0));
}
}

#endif
