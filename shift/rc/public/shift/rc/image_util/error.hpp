#ifndef SHIFT_RC_IMAGE_UTIL_ERROR_H
#define SHIFT_RC_IMAGE_UTIL_ERROR_H

#include <system_error>

namespace shift::rc::image_util
{
enum class error_condition
{
  /// Invalid argument class of errors.
  invalid_argument = 0x01000000,

  /// Internal error class of errors.
  internal_error = 0x02000000,

  /// Operation not supported class of errors.
  operation_not_supported = 0x03000000
};

enum class error_code
{
  // Invalid argument errors:
  /// Source buffer is nullptr.
  source_buffer_null = static_cast<int>(error_condition::invalid_argument),
  /// Destination buffer is nullptr,
  destination_buffer_null,
  /// Source buffer is too small.
  source_buffer_size,
  /// Destination buffer is too small.
  destination_buffer_size,
  /// Source and destination images have different size.
  different_image_size,
  /// Destination region outside image bounds.
  destination_region_bounds,
  /// Source region outside image bounds.
  source_region_bounds,
  /// row_stride in destination image must be zero when using a block compressed
  /// format.
  destination_row_stride_with_bc,
  /// row_stride in source image must be zero when using a block compressed
  /// format.
  source_row_stride_with_bc,
  /// Region source not block aligned.
  source_region_not_block_aligned,
  /// Region destination not block aligned.
  destination_region_not_block_aligned,

  // Internal errors:
  /// Unexpected error converting image.
  unexpected_conversion_error =
    static_cast<int>(error_condition::internal_error),

  // Unsupported operation errors:
  /// Unsupported conversion from source to destination format.
  unsupported_conversion_formats =
    static_cast<int>(error_condition::operation_not_supported),
};

/// Overload of std::make_error_code.
std::error_code make_error_code(error_code code);

/// Overload of std::make_error_condition.
std::error_condition make_error_condition(error_condition condition);
}

namespace std
{
template <>
struct is_error_code_enum<shift::rc::image_util::error_code> : true_type
{
};

template <>
struct is_error_condition_enum<shift::rc::image_util::error_condition> : true_type
{
};
}

#endif
