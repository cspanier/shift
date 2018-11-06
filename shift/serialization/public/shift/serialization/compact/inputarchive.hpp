#ifndef SHIFT_SERIALIZATION_COMPACT_INPUTARCHIVE_HPP
#define SHIFT_SERIALIZATION_COMPACT_INPUTARCHIVE_HPP

#include <shift/core/boost_disable_warnings.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/serialization/compact/compressed.hpp"
#include "shift/serialization/archive.hpp"

namespace shift::serialization
{
///
template <boost::endian::order Order>
class compact_input_archive final
: public archive<boost::iostreams::filtering_istream, Order>
{
public:
  using base_t = archive<boost::iostreams::filtering_istream, Order>;

  /// Constructor.
  compact_input_archive(protocol_version_t protocol_version)
  : archive<boost::iostreams::filtering_istream, Order>(protocol_version)
  {
  }

  /// Destructor.
  ~compact_input_archive() override = default;

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
    base_t::_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(bool&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(char& value)
  {
    base_t::_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(char&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(char16_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    base_t::_stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<order::native, Order>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(char16_t&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(char32_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    base_t::_stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<order::native, Order>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(char32_t&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint8_t& value)
  {
    base_t::_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint8_t&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int8_t& value)
  {
    base_t::_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int8_t&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint16_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    base_t::_stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<order::native, Order>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint16_t&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int16_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    base_t::_stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<order::native, Order>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int16_t&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint32_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    base_t::_stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<order::native, Order>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint32_t&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int32_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    base_t::_stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<order::native, Order>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int32_t&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint64_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    base_t::_stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<order::native, Order>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::uint64_t&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int64_t& value)
  {
    using namespace boost::endian;
    std::decay_t<decltype(value)> source_value;
    base_t::_stream.read(reinterpret_cast<char*>(&source_value), sizeof(value));
    value = conditional_reverse<order::native, Order>(source_value);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::int64_t&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(float& value)
  {
    using namespace boost::endian;
    std::array<std::byte, sizeof(value)> source_value;
    base_t::_stream.read(reinterpret_cast<char*>(source_value.data()),
                         source_value.size());
    if constexpr (Order != order::native)
      std::reverse(source_value.begin(), source_value.end());
    value = *reinterpret_cast<float*>(source_value.data());
    return *this;
  }

  ///
  compact_input_archive& operator>>(float&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(double& value)
  {
    using namespace boost::endian;
    std::array<std::byte, sizeof(value)> source_value;
    base_t::_stream.read(reinterpret_cast<char*>(source_value.data()),
                         source_value.size());
    if constexpr (Order != order::native)
      std::reverse(source_value.begin(), source_value.end());
    value = *reinterpret_cast<double*>(source_value.data());
    return *this;
  }

  ///
  compact_input_archive& operator>>(double&& value)
  {
    base_t::_stream.ignore(sizeof(value));
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::string& value)
  {
    compressed<std::uint32_t> length = 0;
    *this >> length;
    value.resize(length);
    base_t::_stream.read(const_cast<char*>(value.data()), length);
    return *this;
  }

  ///
  compact_input_archive& operator>>(std::string&& /*value*/)
  {
    compressed<std::uint32_t> length = 0;
    *this >> length;
    base_t::_stream.ignore(length);
    return *this;
  }

  /// Read data from the underlying stream.
  inline void read(char* destination, std::size_t size)
  {
    base_t::_stream.read(destination, size);
  }

  /// Read and discard data from the underlying stream.
  inline void ignore(std::size_t size)
  {
    base_t::_stream.ignore(size);
  }
};

///
template <typename U, boost::endian::order Order>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         compressed<U>& value)
{
  U length = 0;
  for (std::size_t i = 0u; i < (sizeof(U) * 8 + 6) / 7; ++i)
  {
    std::uint8_t digit;
    archive >> digit;
    length |= static_cast<U>(digit & 0x7f) << (i * 7);
    if ((digit & 0x80) == 0)
      break;
  }
  value = length;
  return archive;
}
}

#endif
