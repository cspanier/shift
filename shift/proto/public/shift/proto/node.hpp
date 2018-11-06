#ifndef SHIFT_PROTO_NODE_HPP
#define SHIFT_PROTO_NODE_HPP

#include <cstddef>
#include <cstdint>
#include <list>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/variant.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/proto/types.hpp"

namespace shift::proto
{
enum class attribute_assignment
{
  keep,
  overwrite
};

enum class attribute_visibility
{
  internal,
  published
};

/// Base class for all types and data fields.
struct node
{
  node() = default;
  node(const node&) = default;
  node(node&&) = default;
  virtual ~node() = 0;
  node& operator=(const node&) = default;
  node& operator=(node&&) = default;

  ///
  virtual type* find_type(const namescope_path& namescope_path,
                          std::size_t current_depth = 0);

  /// Returns whether an attribute of given name exists.
  bool has_attribute(const std::string& attribute_name) const;

  /// Conditionally overwrites an attribute of given name.
  bool attribute(
    const std::string& attribute_name, std::string value,
    attribute_assignment assignment = attribute_assignment::overwrite,
    attribute_visibility visibility = attribute_visibility::internal);

  bool attribute(
    const std::string& attribute_name, std::uint64_t value,
    attribute_assignment assignment = attribute_assignment::overwrite,
    attribute_visibility visibility = attribute_visibility::internal);

  /// Conditionally overwrites an attribute of given name.
  bool attribute(
    attribute_pair attribute,
    attribute_assignment assignment = attribute_assignment::overwrite,
    attribute_visibility visibility = attribute_visibility::internal);

  /// Returns the value of the attribute of given name.
  /// @exception std::invalid_argument
  ///   There is no attribute of given name.
  template <typename T>
  T attribute(const std::string& attribute_name) const
  {
    auto attribute = public_attributes.find(attribute_name);
    if (attribute == public_attributes.end())
    {
      attribute = internal_attributes.find(attribute_name);
      if (attribute == internal_attributes.end())
      {
        throw std::invalid_argument("attribute \"" + attribute_name +
                                    "\" does not exist in node \"" + name +
                                    "\".");
      }
    }
    if (auto* string = std::get_if<std::string>(&attribute->second))
      return boost::lexical_cast<T>(*string);
    else if (auto* number = std::get_if<std::uint64_t>(&attribute->second))
      return boost::lexical_cast<T>(*number);
    else
    {
      throw std::invalid_argument("attribute \"" + attribute_name +
                                  "\" is invalid in node \"" + name + "\".");
    }
  }

  /// Recursively traverses the AST and generates uid attributes for each node
  /// which doesn't already have one.
  virtual void generate_uids() = 0;

  type* this_type = nullptr;
  std::string name;
  namescope* parent = nullptr;
  std::uint32_t depth = 0;
  attribute_map public_attributes;
  attribute_map internal_attributes;
};
}

#endif
