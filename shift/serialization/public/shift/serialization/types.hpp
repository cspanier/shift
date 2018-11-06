#ifndef SHIFT_SERIALIZATION_TYPES_HPP
#define SHIFT_SERIALIZATION_TYPES_HPP

#include <cstdint>
#include <memory>
#include <shift/core/exception.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/endian/conversion.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::serialization
{
using SerializationErrorInfo =
  boost::error_info<struct TagSerializationErrorInfo, std::string>;

/// An exception type used to report (de-)serialization errors.
struct SerializationError : virtual core::runtime_error
{
};

using protocol_version_t = std::uint32_t;
using message_uid_t = std::uint32_t;

class message;

template <typename Stream, boost::endian::order Order>
class archive;

template <boost::endian::order Order = boost::endian::order::native>
class compact_input_archive;

template <boost::endian::order Order = boost::endian::order::native>
class compact_output_archive;
}

#endif
