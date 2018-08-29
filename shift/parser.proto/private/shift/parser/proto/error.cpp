#include "shift/parser/proto/error.h"

namespace shift::parser::proto
{
std::ostream& operator<<(std::ostream& stream, const error_internal& error)
{
  stream << annotation_source{error.location_info}
         << ": error P1000: internal error\n"
         << annotation_code{error.location_info};
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const error_parse& error)
{
  stream << annotation_source{error.location_info} << ": error P"
         << std::to_string(1100 + error.code)
         << ": parse error: " << error.reason << "\n"
         << annotation_code{error.location_info};
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const error_redefinition& error)
{
  stream << annotation_source{error.token_new}
         << ": error P1001: redefinition of symbol '" << error.token_new.value
         << "'\n"
         << annotation_code{error.token_new} << "\n";
  stream << annotation_source{error.token_old}
         << ": note: see previous definition of '" << error.token_old.value
         << "'\n"
         << annotation_code{error.token_old};
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const error_type_lookup& error)
{
  stream << annotation_source{error.type_path}
         << ": error P1002: cannot find referenced type '" << error.type_path
         << "'\n"
         << annotation_code{error.type_path};
  return stream;
}

std::ostream& operator<<(std::ostream& stream,
                         const error_template_parameter_lookup& error)
{
  stream << annotation_source{error.template_argument}
         << ": error P1003: cannot find referenced template parameter '"
         << error.template_argument << "'\n"
         << annotation_code{error.template_argument};
  return stream;
}
}
