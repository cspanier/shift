#include "shift/parser/proto/tokens.h"
#include <shift/platform/assert.h>

namespace shift::parser::proto
{
// bool type::has_attribute(const std::string& attribute_name) const
//{
//  return public_attributes.find(attribute_name) !=
//           public_attributes.end() ||
//         internal_attributes.find(attribute_name) !=
//           internal_attributes.end();
//}

// bool type::attribute(const std::string& attribute_name,
//                     const std::string& value,
//                     attribute_assignment assignment,
//                     attribute_visibility visibility)
//{
//  if (assignment == attribute_assignment::keep &&
//      has_attribute(attribute_name))
//  {
//    return false;
//  }
//  if (visibility == attribute_visibility::internal)
//    internal_attributes[attribute_name] = std::move(value);
//  else if (visibility == attribute_visibility::published)
//    public_attributes[attribute_name] = std::move(value);
//  else
//    BOOST_ASSERT(false);
//  return true;
//}

// bool type::attribute(const std::string& attribute_name, std::uint64_t value,
//                     attribute_assignment assignment,
//                     attribute_visibility visibility)
//{
//  if (assignment == attribute_assignment::keep &&
//      has_attribute(attribute_name))
//  {
//    return false;
//  }
//  if (visibility == attribute_visibility::internal)
//    internal_attributes[attribute_name] = std::move(value);
//  else if (visibility == attribute_visibility::published)
//    public_attributes[attribute_name] = std::move(value);
//  else
//    BOOST_ASSERT(false);
//  return true;
//}

// bool type::attribute(attribute_pair attribute,
//                     attribute_assignment assignment,
//                     attribute_visibility visibility)
//{
//  if (assignment == attribute_assignment::keep &&
//      has_attribute(attribute.first))
//  {
//    return false;
//  }
//  if (visibility == attribute_visibility::internal)
//    internal_attributes[attribute.first] = std::move(attribute.second);
//  else if (visibility == attribute_visibility::published)
//    public_attributes[attribute.first] = std::move(attribute.second);
//  else
//    BOOST_ASSERT(false);
//  return true;
//}
}
