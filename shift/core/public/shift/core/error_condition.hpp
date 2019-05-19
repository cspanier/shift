#ifndef SHIFT_CORE_ERROR_CONDITION_H
#define SHIFT_CORE_ERROR_CONDITION_H

#include <system_error>

namespace shift::core
{
/// An enumeration of all error classes that may arise throughout all
/// shift libraries.
/// @remarks
///   Each std::error_code returned by any function defined within the shift
///   library collection can be directly compared with these error classes to
///   ease decision of how to handle specific errors.
enum class error_condition
{
  /// Invalid argument class of errors. These errors typically signal wrong API
  /// usage. It might be safe to continue execution. This class of error should
  /// be reported to the developer.
  invalid_argument = 0x01000000,

  /// Operation not supported class of errors. An error from this category could
  /// signal missing hardware support on this computer or hitting an
  /// unimplemented code path. An application should recover from this class of
  /// errors.
  operation_not_supported = 0x02000000,

  /// Internal error class of errors. When getting an error from this category
  /// the program might be in undefined state and should be closed immediately.
  internal_error = 0x03000000
};

/// Overload of std::make_error_condition.
std::error_condition make_error_condition(error_condition condition);
}

namespace std
{
template <>
struct is_error_condition_enum<shift::core::error_condition> : true_type
{
};
}

#endif
