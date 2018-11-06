#ifndef SHIFT_PARSER_PROTO_BUILT_IN_TYPES_HPP
#define SHIFT_PARSER_PROTO_BUILT_IN_TYPES_HPP

#include <memory>
#include "shift/parser/proto/source_module.hpp"

namespace shift::parser::proto
{
/// This type does nothing but holding the static source code of all built-in
/// proto types.
struct built_in_types
{
  static std::shared_ptr<source_module> source;
};
}

#endif
