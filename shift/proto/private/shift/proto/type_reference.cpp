#include "shift/proto/type_reference.hpp"
#include "shift/proto/service.hpp"
#include "shift/proto/interface.hpp"
#include "shift/proto/message.hpp"
#include "shift/proto/enumeration.hpp"
#include "shift/proto/alias.hpp"
#include <shift/core/core.hpp>
#include <shift/platform/assert.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/crc.hpp>
#include <shift/core/boost_restore_warnings.hpp>
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
