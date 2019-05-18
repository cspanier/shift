#ifndef SHIFT_RC_IMAGE_UTIL_LINEAR_IMAGE_VIEW_H
#define SHIFT_RC_IMAGE_UTIL_LINEAR_IMAGE_VIEW_H

#include <cstdint>
#include "shift/rc/image_util/pixel.hpp"
// #include "shift/rc/image_util/pixel_vector.hpp"

namespace shift::rc::image_util
{
/// A view of an image that is stored in linear data format.
template <typename PixelDefinition>
class linear_image_view
{
public:
  using pixel_t = std::remove_const_t<PixelDefinition>;
  using storage_t = std::conditional_t<std::is_const_v<PixelDefinition>,
                                       const std::byte, std::byte>;
  static constexpr std::uint32_t block_width = PixelDefinition::block_width;
  static constexpr std::uint32_t block_height = PixelDefinition::block_height;
  static_assert(block_width == 1 || block_height == 1);

  /// @param buffer
  ///   Pointer to a buffer of pixel data.
  /// @param buffer_size_in_bytes
  ///   Size of the pixel buffer in bytes. This is only used for double
  ///   checking.
  /// @param width
  ///   Image width in pixels.
  /// @param height
  ///   Image height in pixels.
  /// @param row_stride
  ///   Number of bytes between the beginnings of two consecutive rows. When
  ///   this parameter is set to zero a row stride of (width *
  ///   PixelDefinition::size_in_bytes) will be assumed.
  linear_image_view(storage_t* buffer, std::size_t buffer_size_in_bytes,
                    std::uint32_t width, std::uint32_t height,
                    std::uint32_t row_stride = 0)
  : _buffer(buffer),
    _width(width),
    _height(height),
    _row_stride((row_stride > 0) ? row_stride : width * pixel_t::size_in_bytes)
  {
    BOOST_ASSERT(buffer_size_in_bytes >=
                 _width * _height * pixel_t::size_in_bytes);
  }

  /// @returns
  ///   The address of the pixel with the given coordinates.
  storage_t* data(std::uint32_t x, std::uint32_t y) const
  {
    BOOST_ASSERT(x < _width);
    BOOST_ASSERT(y < _height);
    return _buffer + y * _row_stride + x * pixel_t::size_in_bytes;
  }

  void read_pixel(std::uint32_t x, std::uint32_t y, pixel_t& pixel) const
  {
    // Read each pixel channel separately.
    core::for_each<typename pixel_t::channels_t>(
      detail::read_pixel_channel<pixel_t>{}, data(x, y), pixel);
  }

  void write_pixel(std::uint32_t x, std::uint32_t y, const pixel_t& pixel)
  {
    // Write each pixel channel separately.
    core::for_each<typename pixel_t::channels_t>(
      detail::write_pixel_channel<pixel_t>{}, data(x, y), pixel);
  }

private:
  storage_t* _buffer;
  std::uint32_t _width;
  std::uint32_t _height;
  std::uint32_t _row_stride;
};
}

#endif
