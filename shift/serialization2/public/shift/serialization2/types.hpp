#ifndef SHIFT_SERIALIZATION2_TYPES_HPP
#define SHIFT_SERIALIZATION2_TYPES_HPP

#include <cstdint>
#include <memory>
#include <shift/core/exception.hpp>
#include <shift/core/group_ptr.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::serialization2
{
using serialization_error_info =
  boost::error_info<struct tag_serialization_error_info, std::string>;

/// An exception type used to report (de-)serialization errors.
struct serialization_error : virtual core::runtime_error
{
};

///
template <typename T>
union ieee754_to_uint;

template <>
union ieee754_to_uint<float> {
  float foat_value;
  std::uint32_t uint_value;
};

template <>
union ieee754_to_uint<double> {
  double foat_value;
  std::uint64_t uint_value;
};

template <typename Stream, boost::endian::order Order>
class archive;

template <boost::endian::order Order = boost::endian::order::native>
class compact_input_archive;

template <boost::endian::order Order = boost::endian::order::native>
class compact_output_archive;

///
struct begin_read
{
};

///
struct end_read
{
};

///
struct begin_write
{
};

///
struct end_write
{
};

///
struct begin_field
{
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
}

#endif
