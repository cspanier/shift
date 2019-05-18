#include "shift/rc/image_util/error.hpp"
#include <boost/assert.hpp>

namespace shift::rc::image_util
{
///
class error_code_category : std::error_category
{
public:
  const char* name() const noexcept override;

  std::string message(int code) const override;

private:
  error_code_category() = default;

  friend std::error_code make_error_code(error_code code);
};

///
class error_condition_category : std::error_category
{
public:
  const char* name() const noexcept override;

  std::string message(int condition) const override;

  bool equivalent(const std::error_code& code, int condition) const
    noexcept override;

private:
  error_condition_category() = default;

  friend std::error_condition make_error_condition(error_condition condition);
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

const char* error_condition_category::name() const noexcept
{
  return "shift::image::error_condition";
}

std::string error_condition_category::message(int condition) const
{
  switch (static_cast<error_condition>(condition))
  {
  case error_condition::invalid_argument:
    return "Invalid function arguments.";

  case error_condition::internal_error:
    return "Internal error occurred.";

  case error_condition::operation_not_supported:
    return "Operation not supported.";
  }

  BOOST_ASSERT(false);
  return "(unrecognized condition)";
}

bool error_condition_category::equivalent(const std::error_code& code,
                                          int condition) const noexcept
{
  static const auto& code_category = std::error_code{error_code{}}.category();

  switch (static_cast<error_condition>(condition))
  {
  case error_condition::invalid_argument:
    if (code.category() == code_category)
      return code.value() >= 0x100 && code.value() < 0x200;
    return false;

  case error_condition::internal_error:
    if (code.category() == code_category)
      return code.value() >= 0x200 && code.value() < 0x300;
    return false;

  case error_condition::operation_not_supported:
    if (code.category() == code_category)
      return code.value() >= 0x300 && code.value() < 0x400;
    return false;
  }

  BOOST_ASSERT(false);
  return false;
}

std::error_code make_error_code(error_code code)
{
  static const error_code_category category{};
  return {static_cast<int>(code), category};
}

std::error_condition make_error_condition(error_condition condition)
{
  static const error_condition_category category{};
  return {static_cast<int>(condition), category};
}
}
