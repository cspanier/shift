#ifndef SHIFT_LIVEDEBUG_ERROR_CODE_H
#define SHIFT_LIVEDEBUG_ERROR_CODE_H

#include <shift/core/error_condition.hpp>

namespace shift::livedebug
{
///
enum class error_code
{
  // Invalid argument errors:
  // = static_cast<int>(core::error_condition::invalid_argument),

  // Unsupported operation errors:
  cannot_create_socket =
    static_cast<int>(core::error_condition::operation_not_supported),
  endpoint_already_in_use,
  cannot_reuse_address,
  cannot_bind_to_endpoint,
  cannot_listen,

  // Internal errors:
  internal_error = static_cast<int>(core::error_condition::internal_error)
};

/// Overload of std::make_error_code.
std::error_code make_error_code(error_code code);
}

namespace std
{
template <>
struct is_error_code_enum<shift::livedebug::error_code> : true_type
{
};
}

#endif
