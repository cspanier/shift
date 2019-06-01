#ifndef SHIFT_RC_ERROR_CODE_H
#define SHIFT_RC_ERROR_CODE_H

#include <shift/core/error_condition.hpp>

namespace shift::rc
{
///
enum class error_code
{
  // Invalid argument errors:
  /// Source buffer is nullptr.
  source_buffer_null =
    static_cast<int>(core::error_condition::invalid_argument),
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
  /// Source region cannot be empty.
  source_region_empty,
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
  /// The destination format is unsupported.
  destination_format_unsupported,
  /// Source and destination image formats do not match.
  different_image_format,

  // Unsupported operation errors:
  /// This code path has not been implemented, yet.
  not_implemented =
    static_cast<int>(core::error_condition::operation_not_supported),
  /// Unsupported conversion from source to destination format.
  unsupported_conversion_formats,

  // Internal errors:
  /// Unexpected error converting image.
  unexpected_conversion_error =
    static_cast<int>(core::error_condition::internal_error)
};

/// Overload of std::make_error_code.
std::error_code make_error_code(error_code code);
}

namespace std
{
template <>
struct is_error_code_enum<shift::rc::error_code> : true_type
{
};
}

#endif
