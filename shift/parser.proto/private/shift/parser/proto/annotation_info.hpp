#ifndef SHIFT_PARSER_PROTO_ANNOTATIONINFO_HPP
#define SHIFT_PARSER_PROTO_ANNOTATIONINFO_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <ostream>
#include "shift/parser/proto/source_module.hpp"

namespace shift::parser::proto
{
///
struct annotation_info
{
  annotation_info() = default;

  annotation_info(const std::shared_ptr<source_module>& source,
                  std::size_t token_begin, std::size_t token_end)
  : source(source), token_begin(token_begin), token_end(token_end)
  {
  }

  std::weak_ptr<source_module> source;
  std::size_t token_begin;
  std::size_t token_end;
};

///
struct annotation_source
{
  const annotation_info& info;
};

///
std::ostream& operator<<(std::ostream& stream, const annotation_source& where);

///
struct annotation_code
{
  const annotation_info& info;
};

///
std::ostream& operator<<(std::ostream& stream, const annotation_code& where);
}

#endif
