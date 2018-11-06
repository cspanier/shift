#ifndef SHIFT_SERIALIZATION2_COMPACT_OUTPUT_ARCHIVE_HPP
#define SHIFT_SERIALIZATION2_COMPACT_OUTPUT_ARCHIVE_HPP

#include <shift/core/boost_disable_warnings.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/serialization2/compressed.hpp"

namespace shift::serialization2
{
///
template <boost::endian::order Order>
class compact_output_archive final
{
public:
  /// Constructor.
  compact_output_archive(boost::iostreams::filtering_ostream& stream)
  : _stream(stream)
  {
    _stream.exceptions(boost::iostreams::filtering_ostream::failbit |
                       boost::iostreams::filtering_ostream::eofbit);
  }

  compact_output_archive(const compact_output_archive&) = default;
  compact_output_archive(compact_output_archive&&) noexcept = default;

  /// Destructor.
  ~compact_output_archive() = default;

  compact_output_archive& operator=(const compact_output_archive&) = default;
  compact_output_archive& operator=(compact_output_archive&&) noexcept =
    default;

  ///
  compact_output_archive& operator<<(begin_write /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_write /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(begin_field /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_field /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(begin_pair /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_pair /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(begin_tuple /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_tuple /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(begin_array /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_array /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(begin_vector value)
  {
    compressed<std::uint32_t> length = static_cast<std::uint32_t>(value.length);
    BOOST_ASSERT(length == value.length);
    (*this) << length;
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_vector /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(begin_list value)
  {
    *this << begin_vector{value.length};
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_list /*value*/)
  {
    *this << end_vector{};
    return *this;
  }

  ///
  compact_output_archive& operator<<(begin_set value)
  {
    *this << begin_vector{value.length};
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_set /*value*/)
  {
    *this << end_vector{};
    return *this;
  }

  ///
  compact_output_archive& operator<<(begin_map value)
  {
    *this << begin_vector{value.length};
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_map /*value*/)
  {
    *this << end_vector{};
    return *this;
  }

  ///
  compact_output_archive& operator<<(begin_raw_pointer value)
  {
    (*this) << value.valid;
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_raw_pointer /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(begin_shared_pointer value)
  {
    (*this) << value.key;
    return *this;
  }

  ///
  compact_output_archive& operator<<(end_shared_pointer /*value*/)
  {
    // NOP.
    return *this;
  }

  ///
  compact_output_archive& operator<<(bool value)
  {
    write(reinterpret_cast<const char*>(&value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(char value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(char16_t value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(target_value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(char32_t value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(std::uint8_t value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(std::int8_t value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(std::uint16_t value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(std::int16_t value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(std::uint32_t value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(std::int32_t value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(std::uint64_t value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(std::int64_t value)
  {
    using namespace boost::endian;
    auto target_value = conditional_reverse<order::native, Order>(value);
    write(reinterpret_cast<const char*>(&target_value), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(float value)
  {
    ieee754_to_uint<float> temp;
    temp.foat_value = value;
    *this << temp.uint_value;
    return *this;
  }

  ///
  compact_output_archive& operator<<(double value)
  {
    ieee754_to_uint<double> temp;
    temp.foat_value = value;
    *this << temp.uint_value;
    return *this;
  }

  ///
  compact_output_archive& operator<<(std::string value)
  {
    compressed<std::uint32_t> length = static_cast<uint32_t>(value.size());
    BOOST_ASSERT(length == value.size());
    *this << length;
    write(value.c_str(), length);
    return *this;
  }

public:
  /// Write data to the underlying stream.
  void write(const char* source, std::size_t size)
  {
    _stream.write(source, static_cast<std::streamsize>(size));
  }

private:
  boost::iostreams::filtering_ostream& _stream;
};

///
template <boost::endian::order Order, typename U>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const compressed<U>& compressed_value)
{
  auto value = static_cast<U>(compressed_value);
  do
  {
    auto digit = static_cast<std::uint8_t>(value & 0x7F);
    if (value > 0x7F)
      digit |= 0x80;
    archive << digit;
    value >>= 7;
  } while (value > 0);
  return archive;
}
}

#endif
