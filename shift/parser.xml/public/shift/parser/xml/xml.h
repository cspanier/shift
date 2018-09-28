#ifndef SHIFT_PARSER_XML_XML_H
#define SHIFT_PARSER_XML_XML_H

#if defined(SHIFT_PARSER_XML_SHARED)
#if defined(SHIFT_PARSER_XML_EXPORT)
#define SHIFT_PARSER_XML_API SHIFT_EXPORT
#else
#define SHIFT_PARSER_XML_API SHIFT_IMPORT
#endif
#else
#define SHIFT_PARSER_XML_API
#endif

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <shift/core/exception.h>
#include <shift/core/recursive_wrapper.h>

namespace shift::parser::xml
{
/// Exception type used for parsing errors.
struct parse_error : virtual core::runtime_error
{
};

using attribute_t = std::pair<std::string, std::string>;
using attributes_t = std::unordered_map<std::string, std::string>;

enum class node_type
{
  element,
  text,
  comment,
  declaration
};

/// An ultra slim XML node.
class node
{
public:
  /// Erases all child nodes.
  void clear();

  /// Tests if an attribute with the specified name is present.
  /// @param name
  ///   The name of the attribute to query.
  bool has_attribute(std::string attribute_name) const;

  /// Used to query the value of an attribute.
  /// @param name
  ///   The name of the attribute to query.
  /// @throws
  ///   If there is no attribute of the specified name an exception of type
  ///   @see ParserException will be thrown.
  std::string attribute(std::string attribute_name) const;

  /// Used to set the value of an attribute.
  /// @param name
  ///   The name of the attribute to set.
  /// @param value
  ///   The value to assign to the attribute.
  void attribute(std::string attribute_name, std::string value);

  /// Returns the first child in the list with the given tag name.
  node* element_by_name(const std::string& child_name);

  /// Returns the first child in the list with the given tag name.
  const node* element_by_name(const std::string& child_name) const;

  /// This method finds all children with the given tag name and calls the
  /// handler for each occurrence.
  /// @param name
  ///   The tag name of child elements to find.
  /// @param handler
  ///   A callback handler function called once per found element.
  /// @param recursive
  ///   When set to true a recursive depth search through all elements is
  ///   performed.
  bool find_element_by_name(const std::string& child_name,
                            std::function<bool(node&)> handler, bool recursive);

  /// This method finds all children with the given tag name and calls the
  /// handler for each occurrence.
  /// @param name
  ///   The tag name of child elements to find.
  /// @param handler
  ///   A callback handler function called once per found element.
  /// @param recursive
  ///   When set to true a recursive depth search through all elements is
  ///   performed.
  bool find_element_by_name(const std::string& child_name,
                            std::function<bool(const node&)> handler,
                            bool recursive) const;

  ///
  std::string inner_text() const;

public:
  using child_t = core::recursive_wrapper<node>;
  using children_t = std::vector<child_t>;

  node_type type;
  std::string name;
  attributes_t attributes;
  std::string text;
  children_t children;
};

/// Encode reserved characters with XML entity sequences.
std::string encode_entities(std::string text);

/// This operator will read the whole content of stream into a buffer, parse
/// this for XML data and store the result in node.
std::istream& operator>>(std::istream& stream, xml::node& node);

/// Print a XML tree in formatted text form into the passed ostream.
/// @remarks
///   Note that the stream won't get flushed by this function.
/// @remarks
///   Use core::indent_width to control tab width.
std::ostream& operator<<(std::ostream& stream, const xml::node& node);
}

#endif
