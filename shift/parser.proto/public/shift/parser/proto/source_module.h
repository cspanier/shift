#ifndef SHIFT_PARSER_PROTO_SOURCEMODULE_H
#define SHIFT_PARSER_PROTO_SOURCEMODULE_H

#include <cstdint>
#include <string>

namespace shift::parser::proto
{
///
struct source_module
{
  /// Default constructor.
  source_module() = default;

  /// Constructor.
  source_module(std::string_view new_code, std::string_view new_name,
                std::uint32_t new_hash);

  std::string_view code;
  std::string_view name;
  std::uint32_t hash;
};
}

#endif
