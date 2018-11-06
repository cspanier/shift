#ifndef SHIFT_SERIALIZATION_COMPACT_OUTPUTARCHIVE_HPP
#define SHIFT_SERIALIZATION_COMPACT_OUTPUTARCHIVE_HPP

#include <shift/core/boost_disable_warnings.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/serialization/compact/compressed.hpp"
#include "shift/serialization/archive.hpp"

namespace shift::serialization
{
template <boost::endian::order Order>
class compact_output_archive final
: public archive<boost::iostreams::filtering_ostream, Order>
{
public:
  using base_t = archive<boost::iostreams::filtering_ostream, Order>;

  /// Constructor.
  compact_output_archive(protocol_version_t protocol_version)
  : archive<boost::iostreams::filtering_ostream, Order>(protocol_version)
  {
  }

  /// Destructor.
  ~compact_output_archive() override = default;

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
    compressed<std::uint32_t> compressedLength =
      static_cast<std::uint32_t>(value.length);
    BOOST_ASSERT(compressedLength == value.length);
    (*this) << compressedLength;
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
    using namespace boost::endian;
    auto it = reinterpret_cast<std::byte*>(&value);
    if constexpr (Order != order::native)
      std::reverse(it, it + sizeof(value));
    write(reinterpret_cast<const char*>(it), sizeof(value));
    return *this;
  }

  ///
  compact_output_archive& operator<<(double value)
  {
    using namespace boost::endian;
    auto it = reinterpret_cast<std::byte*>(&value);
    if constexpr (Order != order::native)
      std::reverse(it, it + sizeof(value));
    write(reinterpret_cast<const char*>(it), sizeof(value));
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
    base_t::_stream.write(source, size);
  }
};

///
template <typename U, boost::endian::order Order>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const compressed<U>& value)
{
  auto length = static_cast<U>(value);
  do
  {
    auto digit = static_cast<std::uint8_t>(length & 0x7F);
    if (length > 0x7F)
      digit |= 0x80;
    archive << digit;
    length >>= 7;
  } while (length > 0);
  return archive;
}
}

#endif
