#include "shift/proto/alias_type_traits.h"
#include "shift/proto/built_in_type_traits.h"
#include "shift/proto/alias.h"
#include "shift/proto/type_reference.h"
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
bool alias_type_traits::is_char(const alias& type)
{
  if (const auto* built_in_type = type.reference.as_built_in_type())
    return built_in_type_traits::is_char(*built_in_type);
  else if (const auto* alias_type = type.reference.as_alias())
    return alias_type_traits::is_char(*alias_type);
  else
    return false;
}

bool alias_type_traits::is_signed_int(const alias& type)
{
  if (const auto* built_in_type = type.reference.as_built_in_type())
    return built_in_type_traits::is_signed_int(*built_in_type);
  else if (const auto* alias_type = type.reference.as_alias())
    return alias_type_traits::is_signed_int(*alias_type);
  else
    return false;
}

bool alias_type_traits::is_unsigned_int(const alias& type)
{
  if (const auto* built_in_type = type.reference.as_built_in_type())
    return built_in_type_traits::is_unsigned_int(*built_in_type);
  else if (const auto* alias_type = type.reference.as_alias())
    return alias_type_traits::is_unsigned_int(*alias_type);
  else
    return false;
}

bool alias_type_traits::is_int(const alias& type)
{
  if (const auto* built_in_type = type.reference.as_built_in_type())
    return built_in_type_traits::is_int(*built_in_type);
  else if (const auto* alias_type = type.reference.as_alias())
    return alias_type_traits::is_int(*alias_type);
  else
    return false;
}

bool alias_type_traits::is_float(const alias& type)
{
  if (const auto* built_in_type = type.reference.as_built_in_type())
    return built_in_type_traits::is_float(*built_in_type);
  else if (const auto* alias_type = type.reference.as_alias())
    return alias_type_traits::is_float(*alias_type);
  else
    return false;
}

bool alias_type_traits::is_string(const alias& type)
{
  if (const auto* built_in_type = type.reference.as_built_in_type())
    return built_in_type_traits::is_string(*built_in_type);
  else if (const auto* alias_type = type.reference.as_alias())
    return alias_type_traits::is_string(*alias_type);
  else
    return false;
}

bool alias_type_traits::is_template(const alias& type)
{
  if (const auto* built_in_type = type.reference.as_built_in_type())
    return built_in_type_traits::is_template(*built_in_type);
  else if (const auto* alias_type = type.reference.as_alias())
    return alias_type_traits::is_template(*alias_type);
  else
    return false;
}
}
