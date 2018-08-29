#include "shift/proto/built_in_type_traits.h"
#include "shift/proto/hash.h"
#include <shift/core/core.h>
#include <shift/platform/assert.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/crc.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <algorithm>
#include <cstdint>
#include <string>

namespace shift::proto
{
bool built_in_type_traits::is_char(built_in_type type)
{
  return type == built_in_type::char8 || type == built_in_type::char16 ||
         type == built_in_type::char32;
}

bool built_in_type_traits::is_signed_int(built_in_type type)
{
  return type == built_in_type::int8 || type == built_in_type::int16 ||
         type == built_in_type::int32 || type == built_in_type::int64;
}

bool built_in_type_traits::is_unsigned_int(built_in_type type)
{
  return type == built_in_type::uint8 || type == built_in_type::uint16 ||
         type == built_in_type::uint32 || type == built_in_type::uint64 ||
         type == built_in_type::datetime;
}

bool built_in_type_traits::is_int(built_in_type type)
{
  return is_signed_int(type) || is_unsigned_int(type);
}

bool built_in_type_traits::is_float(built_in_type type)
{
  return type == built_in_type::float32 || type == built_in_type::float64;
}

bool built_in_type_traits::is_string(built_in_type type)
{
  return type == built_in_type::string;
}

bool built_in_type_traits::is_template(built_in_type type)
{
  return type == built_in_type::raw_ptr || type == built_in_type::unique_ptr ||
         type == built_in_type::shared_ptr ||
         type == built_in_type::group_ptr || type == built_in_type::tuple ||
         type == built_in_type::array || type == built_in_type::list ||
         type == built_in_type::vector || type == built_in_type::set ||
         type == built_in_type::matrix || type == built_in_type::map ||
         type == built_in_type::variant || type == built_in_type::bitfield;
}
}
