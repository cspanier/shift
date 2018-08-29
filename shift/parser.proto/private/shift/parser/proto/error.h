#ifndef SHIFT_PARSER_PROTO_ERROR_H
#define SHIFT_PARSER_PROTO_ERROR_H

#include <cstdint>
#include <string>
#include <ostream>
#include "shift/parser/proto/tokens.h"

namespace shift::parser::proto
{
///
struct error_internal
{
  friend std::ostream& operator<<(std::ostream& stream,
                                  const error_internal& error);

  const annotation_info& location_info;
};

///
struct error_parse
{
  friend std::ostream& operator<<(std::ostream& stream,
                                  const error_parse& error);

  std::string reason;
  std::size_t code;
  annotation_info location_info;
};

///
struct error_redefinition
{
  friend std::ostream& operator<<(std::ostream& stream,
                                  const error_redefinition& error);

  const string_token& token_new;
  const string_token& token_old;
};

///
struct error_type_lookup
{
  friend std::ostream& operator<<(std::ostream& stream,
                                  const error_type_lookup& error);

  const type_path_token& type_path;
};

///
struct error_template_parameter_lookup
{
  friend std::ostream& operator<<(std::ostream& stream,
                                  const error_template_parameter_lookup& error);

  const string_token& template_argument;
};
}

#endif
