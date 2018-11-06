#include "shift/proto/node.hpp"
#include "shift/proto/hash.hpp"
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
node::~node() = default;

type* node::find_type(const namescope_path& /*namescope_path*/,
                      std::size_t /*current_depth*/)
{
  BOOST_ASSERT(false);
  return nullptr;
}

bool node::has_attribute(const std::string& attribute_name) const
{
  return public_attributes.find(attribute_name) != public_attributes.end() ||
         internal_attributes.find(attribute_name) != internal_attributes.end();
}

bool node::attribute(const std::string& attribute_name, std::string value,
                     attribute_assignment assignment,
                     attribute_visibility visibility)
{
  if (assignment == attribute_assignment::keep && has_attribute(attribute_name))
  {
    return false;
  }
  if (visibility == attribute_visibility::internal)
    internal_attributes[attribute_name] = std::move(value);
  else if (visibility == attribute_visibility::published)
    public_attributes[attribute_name] = std::move(value);
  else
    BOOST_ASSERT(false);
  return true;
}

bool node::attribute(const std::string& attribute_name, std::uint64_t value,
                     attribute_assignment assignment,
                     attribute_visibility visibility)
{
  if (assignment == attribute_assignment::keep && has_attribute(attribute_name))
  {
    return false;
  }
  if (visibility == attribute_visibility::internal)
    internal_attributes[attribute_name] = value;
  else if (visibility == attribute_visibility::published)
    public_attributes[attribute_name] = value;
  else
    BOOST_ASSERT(false);
  return true;
}

bool node::attribute(attribute_pair attribute, attribute_assignment assignment,
                     attribute_visibility visibility)
{
  if (assignment == attribute_assignment::keep &&
      has_attribute(attribute.first))
  {
    return false;
  }
  if (visibility == attribute_visibility::internal)
    internal_attributes[attribute.first] = std::move(attribute.second);
  else if (visibility == attribute_visibility::published)
    public_attributes[attribute.first] = std::move(attribute.second);
  else
    BOOST_ASSERT(false);
  return true;
}
}
