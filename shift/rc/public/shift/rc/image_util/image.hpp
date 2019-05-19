#ifndef SHIFT_RC_IMAGE_UTIL_IMAGE_H
#define SHIFT_RC_IMAGE_UTIL_IMAGE_H

#include <system_error>
#include <shift/resource_db/image.hpp>
#include "shift/rc/error_code.hpp"

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
};

struct source_image_descriptor : image_descriptor
{
  /// The number of bytes available in the image buffer.
  std::size_t buffer_size;
  /// A pointer to a buffer from where to load an image.
  const std::byte* buffer;
};

struct destination_image_descriptor : image_descriptor
{
  /// The number of bytes available in the image buffer.
  std::size_t buffer_size;
  /// A pointer to a buffer where to store an image.
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

/// This function calculates the minimum required amount of memory in number of
/// bytes that an image of given dimensions and format.
std::size_t required_image_buffer_size(const image_descriptor& image);

/// Converts the content of am image to a different format.
/// @param destination_image
///   The descriptor of the image to write to. The destination buffer has to be
///   already allocated. Use required_image_buffer_size to create a buffer of
///   appropriate size.
std::error_code convert_image(
  const destination_image_descriptor& destination_image,
  const source_image_descriptor& source_image, const convert_region& region);
}

#endif
