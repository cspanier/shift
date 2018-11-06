#ifndef SHIFT_SERIALIZATION2_COMPACT_INPUT_ARCHIVE_HPP
#define SHIFT_SERIALIZATION2_COMPACT_INPUT_ARCHIVE_HPP

#include <shift/core/boost_disable_warnings.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/serialization2/compressed.hpp"

namespace shift::serialization2
{
///
template <boost::endian::order Order>
class compact_input_archive final
{
public:
  /// Constructor.
  compact_input_archive(boost::iostreams::filtering_istream& stream)
  : _stream(stream)
  {
    _stream.exceptions(boost::iostreams::filtering_istream::failbit |
                       boost::iostreams::filtering_istream::eofbit);
  }

  /// Destructor.
  ~compact_input_archive() = default;

public:
  ///
  compact_input_archive& operator>>(begin_read& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_read&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_read& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_read&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_field& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_field&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_field& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_field&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_pair& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_pair&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_pair& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_pair&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_tuple& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_tuple&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_tuple& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_tuple&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_array& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_array&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_array& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_array&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_vector& value)
  {
    compressed<std::uint32_t> length;
    (*this) >> length;
    value.length = length;
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_vector&& value)
  {
    compressed<std::uint32_t> length;
    (*this) >> length;
    value.length = length;
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_vector& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_vector&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_list& value)
  {
    compressed<std::uint32_t> length;
    (*this) >> length;
    value.length = length;
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_list&& value)
  {
    compressed<std::uint32_t> length;
    (*this) >> length;
    value.length = length;
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_list& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_list&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_set& value)
  {
    compressed<std::uint32_t> length;
    (*this) >> length;
    value.length = length;
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_set&& value)
  {
    compressed<std::uint32_t> length;
    (*this) >> length;
    value.length = length;
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_set& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_set&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_map& value)
  {
    compressed<std::uint32_t> length;
    (*this) >> length;
    value.length = length;
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_map&& value)
  {
    compressed<std::uint32_t> length;
    (*this) >> length;
    value.length = length;
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_map& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_map&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_raw_pointer& value)
  {
    (*this) >> value.valid;
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_raw_pointer&& value)
  {
    (*this) >> value.valid;
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_raw_pointer& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_raw_pointer&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_shared_pointer& value)
  {
    (*this) >> value.key;
    return *this;
  }

  ///
  compact_input_archive& operator>>(begin_shared_pointer&& value)
  {
    (*this) >> value.key;
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_shared_pointer& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(end_shared_pointer&& /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_input_archive& operator>>(bool& value)
  {
    _stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(bool&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(char& value)
  {
    _stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(char&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(char16_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    _stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<Order, order::native>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(char16_t&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(char32_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    _stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<Order, order::native>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(char32_t&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint8_t& value)
  {
    _stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint8_t&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int8_t& value)
  {
    _stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int8_t&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint16_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    _stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<Order, order::native>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint16_t&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int16_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    _stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<Order, order::native>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int16_t&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint32_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    _stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<Order, order::native>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint32_t&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int32_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    _stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<Order, order::native>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int32_t&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint64_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    _stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<Order, order::native>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint64_t&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int64_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    _stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<Order, order::native>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int64_t&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(float& value)
  {
    ieee754_to_uint<float> source_value;
    *this >> source_value.uint_value;
    value = source_value.foat_value;
    return *this;
  }

  ///
  compact_input_archive& operator>>(float&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(double& value)
  {
    ieee754_to_uint<double> source_value;
    *this >> source_value.uint_value;
    value = source_value.foat_value;
    return *this;
  }

  ///
  compact_input_archive& operator>>(double&& value)
  {
    _stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::string& value)
  {
    compressed<std::uint32_t> length = 0;
    *this >> length;
    value.resize(length);
    _stream.read(const_cast<char*>(value.data()), length);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::string&& /*value*/)
  {
    compressed<std::uint32_t> length = 0;
    *this >> length;
    _stream.ignore(length);
    return *this;
  }

  /// Read data from the underlying stream.
  inline void read(char* destination, std::size_t size)
  {
    _stream.read(destination, static_cast<std::streamsize>(size));
  }

  /// Read and discard data from the underlying stream.
  inline void ignore(std::size_t size)
  {
    _stream.ignore(static_cast<std::streamsize>(size));
  }

private:
  boost::iostreams::filtering_istream& _stream;
};

///
template <boost::endian::order Order, typename U>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         compressed<U>& compressed_value)
{
  U value = 0;
  for (std::size_t i = 0u; i < (sizeof(U) * 8 + 6) / 7; ++i)
  {
    std::uint8_t digit;
    archive >> digit;
    value |= static_cast<U>(digit & 0x7f) << (i * 7);
    if ((digit & 0x80) == 0)
      break;
  }
  compressed_value = value;
  return archive;
}
}

#endif
