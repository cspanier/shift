#include "shift/parser/proto/source_module.h"

namespace shift::parser::proto
{
source_module::source_module(std::string_view new_code,
                             std::string_view new_name, std::uint32_t new_hash)
: code(new_code), name(new_name), hash(new_hash)
{
}
}
