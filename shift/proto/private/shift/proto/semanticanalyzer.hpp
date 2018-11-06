#ifndef SHIFT_PROTO_SEMANTICANALYZER_HPP
#define SHIFT_PROTO_SEMANTICANALYZER_HPP

#include "shift/proto/types.hpp"

namespace shift::proto
{
/// Task of the semantic analyzer is to check for semantic errors within the
/// abstract syntax tree (AST).
class semantic_analyzer
{
public:
  ///
  bool analyze_namescope(namescope& namescope) const;

private:
  ///
  bool resolve_type(namescope& namescope, type_reference& type_reference) const;
};
}

#endif
