#include "shift/proto/type_reference.h"
#include "shift/proto/service.h"
#include "shift/proto/interface.h"
#include "shift/proto/message.h"
#include "shift/proto/enumeration.h"
#include "shift/proto/alias.h"
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
const proto::built_in_type* type_reference::as_built_in_type(
  bool recursive) const
{
  if (recursive)
  {
    if (auto other = std::get_if<const alias*>(&variant))
      return (*other)->reference.as_built_in_type(recursive);
  }
  return std::get_if<proto::built_in_type>(&variant);
}

const alias* type_reference::as_alias() const
{
  if (auto result = std::get_if<const alias*>(&variant))
    return *result;
  else
    return nullptr;
}

const enumeration* type_reference::as_enumeration(bool recursive) const
{
  if (recursive)
  {
    if (auto other = std::get_if<const alias*>(&variant))
      return (*other)->reference.as_enumeration(recursive);
  }
  if (auto result = std::get_if<const proto::enumeration*>(&variant))
    return *result;
  else
    return nullptr;
}

const message* type_reference::as_message(bool recursive) const
{
  if (recursive)
  {
    if (auto other = std::get_if<const alias*>(&variant))
      return (*other)->reference.as_message(recursive);
  }
  if (auto result = std::get_if<const proto::message*>(&variant))
    return *result;
  else
    return nullptr;
}
}
