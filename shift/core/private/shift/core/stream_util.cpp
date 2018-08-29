#include "shift/core/stream_util.h"
#include <shift/platform/assert.h>

namespace shift::core
{
namespace implementation
{
  int indent_level_index()
  {
    static int index = std::ios_base::xalloc();
    return index;
  }

  int indent_character_index()
  {
    static int index = std::ios_base::xalloc();
    return index;
  }

  int indent_width_index()
  {
    static int index = std::ios_base::xalloc();
    return index;
  }

  void indent_character(std::ios_base& stream, int character)
  {
    stream.iword(implementation::indent_character_index()) = character;
  }

  void indent_width(std::ios_base& stream, int width)
  {
    stream.iword(implementation::indent_width_index()) = width;
  }
}

std::ios_base& inc_indent(std::ios_base& stream)
{
  ++stream.iword(implementation::indent_level_index());
  return stream;
}

std::ios_base& dec_indent(std::ios_base& stream)
{
  auto& level = stream.iword(implementation::indent_level_index());
  if (level > 0)
    --level;
  return stream;
}
}
