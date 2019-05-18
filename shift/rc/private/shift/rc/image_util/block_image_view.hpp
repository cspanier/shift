#ifndef SHIFT_RC_IMAGE_UTIL_BLOCK_IMAGE_VIEW_H
#define SHIFT_RC_IMAGE_UTIL_BLOCK_IMAGE_VIEW_H

#include <cstdint>
#include "shift/rc/image_util/pixel.hpp"
// #include "shift/rc/image_util/pixel_vector.hpp"

namespace shift::rc::image_util
{
/// A view of an image that is stored in a block compressed data format.
template <typename PixelDefinition>
class block_image_view
{
public:
  using pixel_t = std::remove_const_t<PixelDefinition>;
  using storage_t = std::conditional_t<std::is_const_v<PixelDefinition>,
                                       const std::byte, std::byte>;
  using uncompressed_pixel_t = pixel_definition_t<
    uncompressed_pixel_component_t<typename pixel_t::channels_t>,
    pixel_t::color_space,
    uncompressed_pixel_channels_t<typename pixel_t::channels_t>>;
  static constexpr std::uint32_t block_width = PixelDefinition::block_width;
  static constexpr std::uint32_t block_height = PixelDefinition::block_height;
  static_assert(block_width > 1 || block_height > 1);
  using pixel_block_t =
    std::array<std::byte, block_width * block_height * pixel_t::size_in_bytes>;

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
  ///   Unused parameter for block compressed image views.
  block_image_view(storage_t* buffer, std::size_t buffer_size_in_bytes,
                   std::uint32_t width, std::uint32_t height,
                   std::uint32_t /*row_stride*/ = 0)
  : _buffer(buffer), _width(width), _height(height)
  {
    BOOST_ASSERT(buffer_size_in_bytes >=
                 ((_width + block_width - 1) / block_width) *
                   ((_height + block_height - 1) / block_height) *
                   pixel_t::size_in_bytes);
  }

  /// @returns
  ///   The address of the pixel block with the given coordinates.
  /// @param x
  ///   The block's x coordinate must be a multiple of block_width.
  /// @param y
  ///   The block's y coordinate must be a multiple of block_height.
  std::byte* data(std::uint32_t x, std::uint32_t y) const
  {
    BOOST_ASSERT(x < _width);
    BOOST_ASSERT(y < _height);
    BOOST_ASSERT(x % block_width == 0);
    BOOST_ASSERT(y % block_height == 0);
    auto blocks_per_row = ((_width + block_width - 1) / block_width);
    return _buffer + ((y / block_height) * blocks_per_row + (x / block_width)) *
                       pixel_t::size_in_bytes;
  }

  void read_pixel(std::uint32_t /*x*/, std::uint32_t /*y*/,
                  pixel_t& /*pixel*/) const
  {
    /// ToDo: ...
    //    // Read each pixel channel separately.
    //    core::for_each<typename pixel_t::channels_t>(
    //      detail::read_pixel_channel<pixel_t>{}, data(x, y),
    //      pixel);
  }

  /// @param x
  ///   The block's x coordinate must be a multiple of block_width.
  /// @param y
  ///   The block's y coordinate must be a multiple of block_height.
  void write_pixel_block(std::uint32_t /*x*/, std::uint32_t /*y*/,
                         std::uint32_t /*pixel_mask*/,
                         const pixel_block_t& /*pixel_block*/)
  {
    /// ToDo: Compress pixel block and copy the result to data(x, y).
    // std::memcpy(data(x, y), compressed_pixel_block.data(),
    //             pixel_t::size_in_bytes);
  }

private:
  storage_t* _buffer;
  std::uint32_t _width;
  std::uint32_t _height;
};
}

#endif
