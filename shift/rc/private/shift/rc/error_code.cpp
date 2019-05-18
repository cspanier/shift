#include "shift/rc/error_code.hpp"
#include <boost/assert.hpp>

namespace shift::rc
{
///
class error_code_category : std::error_category
{
public:
  const char* name() const noexcept override;

  std::string message(int code) const override;

  bool equivalent(int code, const std::error_condition& condition) const
    noexcept override;

private:
  error_code_category() = default;

  friend std::error_code make_error_code(error_code code);
};

const char* error_code_category::name() const noexcept
{
  return "shift::image::error_code";
}

std::string error_code_category::message(int code) const
{
  switch (static_cast<error_code>(code))
  {
  case error_code::source_buffer_null:
    return "Pointer to source buffer is nullptr.";
  case error_code::destination_buffer_null:
    return "Pointer to destination buffer is nullptr.";
  case error_code::source_buffer_size:
    return "Source buffer is too small.";
  case error_code::destination_buffer_size:
    return "Destination buffer is too small.";
  case error_code::different_image_size:
    return "Source and destination images have different size.";
  case error_code::destination_region_bounds:
    return "Destination region outside image bounds.";
  case error_code::source_region_bounds:
    return "Source region outside image bounds.";
  case error_code::destination_row_stride_with_bc:
    return "The field row_stride in destination image must be zero when using "
           "a block compressed format.";
  case error_code::source_row_stride_with_bc:
    return "The field row_stride in source image must be zero when using a "
           "block compressed format.";
  case error_code::source_region_not_block_aligned:
    return "Source region is not aligned to block size of source pixel format.";
  case error_code::destination_region_not_block_aligned:
    return "Destination region is not aligned to block size of destination "
           "pixel format.";

  case error_code::unexpected_conversion_error:
    return "Unexpected internal error while converting image.";

  case error_code::unsupported_conversion_formats:
    return "Unsupported conversion from source to destination format.";
  }

  BOOST_ASSERT(false);
  return "(unrecognized error)";
}

bool error_code_category::equivalent(
  int code, const std::error_condition& condition) const noexcept
{
  using shift::core::error_condition;

  static const auto& error_condition_category =
    std::error_condition{error_condition{}}.category();

  if (condition.category() != error_condition_category)
    return false;

  switch (static_cast<error_condition>(condition.value()))
  {
  case error_condition::invalid_argument:
    return code >= static_cast<int>(error_condition::invalid_argument) &&
           code < static_cast<int>(error_condition::operation_not_supported);

  case error_condition::operation_not_supported:
    return code >= static_cast<int>(error_condition::operation_not_supported) &&
           code < static_cast<int>(error_condition::internal_error);

  case error_condition::internal_error:
    return code >= static_cast<int>(error_condition::internal_error);
  }

  BOOST_ASSERT(false);
  return false;
}

std::error_code make_error_code(error_code code)
{
  static const error_code_category category{};
  return {static_cast<int>(code), category};
}
}
