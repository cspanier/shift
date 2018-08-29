#include "shift/parser/xml/xml.h"
#include <shift/core/stream_util.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/algorithm/string/replace.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <array>

#if defined(SHIFT_PLATFORM_WINDOWS)
#define br "\r\n"
#else
#define br "\n"
#endif
#define br2 br br

namespace shift::parser::xml
{
using core::inc_indent;
using core::dec_indent;
using core::indent;

/// Encode XML reserved characters like less than and ampersand characters
/// to their corresponding entity references "&lt;" and "&amp;"
std::string encode_entities(std::string text)
{
  static const std::array<std::pair<std::string, std::string>, 5> entities = {{
    {"&", "&amp;"},
    {"<", "&lt;"},
    {">", "&gt;"},
    {"'", "&apos;"},
    {"\"", "&quot;"},
  }};
  for (const auto& entity : entities)
    text = boost::replace_all_copy(text, entity.first, entity.second);
  return text;
}

std::ostream& operator<<(std::ostream& stream, const xml::node& node)
{
  if (node.children.empty())
    return stream;
  for (const auto& child : node.children)
  {
    switch (child->type)
    {
    case node_type::element:
    {
      if (child->name == "xs:annotation")
        return stream;
      stream << core::indent << '<' << child->name << core::inc_indent;
      for (auto& attribute : child->attributes)
      {
        stream << br << core::indent << attribute.first << "=\""
               << encode_entities(attribute.second) << '"';
      }
      if (!child->children.empty())
      {
        stream << ">" br;
        for (const auto& inner_child : child->children)
          stream << *inner_child;
        stream << core::dec_indent << core::indent << "</" << child->name
               << ">" br;
      }
      else
        stream << " />" br << core::dec_indent;
      break;
    }
    case node_type::text:
    {
      stream << core::indent << encode_entities(child->text) << br;
      break;
    }
    case node_type::comment:
    {
      stream << core::indent << "<!--" << encode_entities(child->text)
             << "-->" br;
      break;
    }
    case node_type::declaration:
    {
      stream << core::indent << "<?xml";
      for (auto& attribute : child->attributes)
      {
        stream << br << core::inc_indent << core::indent << attribute.first
               << "=\"" << encode_entities(attribute.second) << '"'
               << core::dec_indent;
      }
      stream << "?>" br;
      break;
    }
    default:
      BOOST_ASSERT(false);
    }
  }
  return stream;
}
}
