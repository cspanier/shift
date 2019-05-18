#ifndef SHIFT_RC_IMAGE_UTIL_IMAGE_H
#define SHIFT_RC_IMAGE_UTIL_IMAGE_H

#include <system_error>
#include <shift/resource_db/image.hpp>
#include <shift/rc/image_util/error.hpp>

namespace shift::rc::image_util
{
struct image_descriptor
{
  /// Number of pixels along the X-axis.
  std::uint32_t width;
  /// Number of pixels along the Y-axis.
  std::uint32_t height;
  /// An optional distance in bytes between the start of two sequential rows.
  /// When format is assigned a block compressed format this must be zero.
  /// Otherwise, when set to zero, it is assumed that the distance is width *
  /// size_of_pixel. When set to a non-zero value it must not be smaller than
  /// width * size_of_pixel.
  std::uint32_t row_stride;
  /// The image format.
  resource_db::image_format format;
  /// The number of bytes available in the image buffer.
  std::size_t buffer_size;
};

struct source_image_descriptor : image_descriptor
{
  const std::byte* buffer;
};

struct destination_image_descriptor : image_descriptor
{
  std::byte* buffer;
};

struct convert_region
{
  std::uint32_t destination_x;
  std::uint32_t destination_y;
  std::uint32_t source_x;
  std::uint32_t source_y;
  std::uint32_t width;
  std::uint32_t height;
};

std::error_code convert_image(
  const destination_image_descriptor& destination_image,
  const source_image_descriptor& source_image, const convert_region& region);
}

#endif
