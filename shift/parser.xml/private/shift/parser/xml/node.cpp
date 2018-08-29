#include "shift/parser/xml/xml.h"
#include "shift/parser/xml/grammar.h"

namespace shift::parser::xml
{
void node::clear()
{
  children.clear();
}

bool node::has_attribute(const std::string attribute_name) const
{
  return attributes.find(attribute_name) != attributes.end();
}

std::string node::attribute(const std::string attribute_name) const
{
  auto attributeIterator = attributes.find(attribute_name);
  if (attributeIterator != attributes.end())
    return attributeIterator->second;
  else
    return {};
}

void node::attribute(const std::string attribute_name, const std::string value)
{
  attributes[attribute_name] = value;
}

node* node::element_by_name(const std::string& child_name)
{
  for (auto& child : children)
  {
    if (child->type == node_type::element)
    {
      if (child->name == child_name)
        return child.get();
    }
  }
  return nullptr;
}

const node* node::element_by_name(const std::string& child_name) const
{
  for (const auto& child : children)
  {
    if (child->type == node_type::element)
    {
      if (child->name == child_name)
        return child.get();
    }
  }
  return nullptr;
}

bool node::find_element_by_name(const std::string& child_name,
                                std::function<bool(node&)> handler,
                                bool recursive)
{
  for (auto& child : children)
  {
    if (child->type == node_type::element)
    {
      if (child->name == child_name)
      {
        if (!handler(*child))
          return false;
      }
      if (recursive)
      {
        if (!child->find_element_by_name(child_name, handler, recursive))
          return false;
      }
    }
  }
  return true;
}

bool node::find_element_by_name(const std::string& child_name,
                                std::function<bool(const node&)> handler,
                                bool recursive) const
{
  for (const auto& child : children)
  {
    if (child->type == node_type::element)
    {
      if (child->name == child_name)
      {
        if (!handler(*child))
          return false;
      }
      if (recursive)
      {
        if (!child->find_element_by_name(child_name, handler, recursive))
          return false;
      }
    }
  }
  return true;
}

std::string node::inner_text() const
{
  std::stringstream result;
  for (const auto& child : children)
  {
    if (child->type == node_type::text)
      result << child->text;
    else if (child->type == node_type::element)
      result << child->inner_text();
  }
  return result.str();
}

std::istream& operator>>(std::istream& stream, xml::node& node)
{
  node.clear();

  std::string content;              // We will read the contents here.
  stream.unsetf(std::ios::skipws);  // No white space skipping!
  std::copy(std::istream_iterator<char>(stream), std::istream_iterator<char>(),
            std::back_inserter(content));

  if (!phrase_parse(content.cbegin(), content.cend(), grammar{}, xml_skipper{},
                    node.children))
  {
    /// ToDo: Improve error handling.
    BOOST_THROW_EXCEPTION(parse_error());
  }
  return stream;
}
}
