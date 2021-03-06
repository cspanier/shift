#ifndef SHIFT_RC_IMAGE_UTIL_BLOCK_IMAGE_VIEW_H
#define SHIFT_RC_IMAGE_UTIL_BLOCK_IMAGE_VIEW_H

#include <cstdint>
#include <squish.h>
#include "shift/rc/image_util/pixel.hpp"
// #include "shift/rc/image_util/pixel_vector.hpp"
#include "shift/rc/image_util/convert.hpp"

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
  static constexpr std::uint32_t block_width = PixelDefinition::block_width;
  static constexpr std::uint32_t block_height = PixelDefinition::block_height;
  static_assert(block_width > 1 || block_height > 1);
  using pixel_block_t = std::array<std::byte, pixel_t::size_in_bytes>;

  using uncompressed_pixel_t = pixel_definition_t<
    uncompressed_pixel_component_t<pixel_t::data_type,
                                   typename pixel_t::channels_t>,
    pixel_t::data_type,
    uncompressed_pixel_channels_t<typename pixel_t::channels_t>>;
  static_assert(!uncompressed_pixel_t::is_block_format);
  using uncompressed_pixel_block_t =
    std::array<uncompressed_pixel_t, block_width * block_height>;

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

    if constexpr (std::is_same_v<pixel_t, pixel_bc1_rgb_unorm> ||
                  std::is_same_v<pixel_t, pixel_bc1_rgb_srgb> ||
                  std::is_same_v<pixel_t, pixel_bc1_rgba_unorm> ||
                  std::is_same_v<pixel_t, pixel_bc1_rgba_srgb>)
    {
      _flags = squish::squish_flag::compression_bc1;
    }
    else if constexpr (std::is_same_v<pixel_t, pixel_bc2_unorm> ||
                       std::is_same_v<pixel_t, pixel_bc2_srgb>)
    {
      _flags = squish::squish_flag::compression_bc2;
    }
    else if constexpr (std::is_same_v<pixel_t, pixel_bc3_unorm> ||
                       std::is_same_v<pixel_t, pixel_bc3_srgb>)
    {
      _flags = squish::squish_flag::compression_bc3;
    }
    else if constexpr (std::is_same_v<pixel_t, pixel_bc4_unorm> ||
                       std::is_same_v<pixel_t, pixel_bc4_snorm>)
    {
      _flags = squish::squish_flag::compression_bc4;
    }
    else if constexpr (std::is_same_v<pixel_t, pixel_bc5_unorm> ||
                       std::is_same_v<pixel_t, pixel_bc5_snorm>)
    {
      _flags = squish::squish_flag::compression_bc5;
    }
    else if constexpr (std::is_same_v<pixel_t, pixel_bc6h_sfloat> ||
                       std::is_same_v<pixel_t, pixel_bc6h_ufloat>)
    {
      _flags = squish::squish_flag::compression_bc6;
    }
    else if constexpr (std::is_same_v<pixel_t, pixel_bc7_unorm> ||
                       std::is_same_v<pixel_t, pixel_bc7_srgb>)
    {
      _flags = squish::squish_flag::compression_bc7;
    }
    else
      BOOST_ASSERT(false);

    if constexpr (std::is_same_v<pixel_t, pixel_bc1_rgb_srgb> ||
                  std::is_same_v<pixel_t, pixel_bc1_rgba_srgb> ||
                  std::is_same_v<pixel_t, pixel_bc2_srgb> ||
                  std::is_same_v<pixel_t, pixel_bc3_srgb> ||
                  std::is_same_v<pixel_t, pixel_bc7_srgb>)
    {
      _flags |= squish::squish_flag::option_srgbness;
    }
  }

  /// @returns
  ///   The address of the pixel block with the given coordinates.
  /// @param block_x
  ///   The n-th block on the x-axis.
  /// @param block_y
  ///   The n-th block on the y-axis.
  storage_t* data(std::uint32_t block_x, std::uint32_t block_y) const
  {
    BOOST_ASSERT(block_x < (_width + block_width - 1) / block_width);
    BOOST_ASSERT(block_y < (_height + block_height - 1) / block_height);
    auto blocks_per_row = ((_width + block_width - 1) / block_width);
    return _buffer +
           (block_y * blocks_per_row + block_x) * pixel_t::size_in_bytes;
  }

  /// Reads and uncompresses a block of pixel.
  /// @param block_x
  ///   The n-th block on the x-axis.
  /// @param block_y
  ///   The n-th block on the y-axis.
  void read_pixel_block(std::uint32_t x, std::uint32_t y,
                        uncompressed_pixel_block_t& pixels) const
  {
    BOOST_ASSERT(x % block_width == 0);
    BOOST_ASSERT(y % block_height == 0);

    pixel_block_t pixel_block;
    std::memcpy(pixel_block.data(), data(x / block_width, y / block_height),
                pixel_t::size_in_bytes);
    squish::decompress(&pixels[0][0], pixel_block.data(), _flags);
  }

  void read_pixel(std::uint32_t x, std::uint32_t y,
                  uncompressed_pixel_t& pixel) const
  {
    // static pixel_converter<pixel_t, uncompressed_pixel_t> converter;

    uncompressed_pixel_block_t block;
    read_pixel_block(x / block_width * block_width,
                     y / block_height * block_height, block);
    pixel = block[y % block_height * block_width + x % block_width];
    // converter(pixel, block[y % block_height * block_width + x %
    // block_width]);
  }

  //  void read_pixel(std::uint32_t x, std::uint32_t y, pixel_t& pixel)
  //  {
  //    static pixel_converter<pixel_t, uncompressed_pixel_t> converter;

  //    auto block_x = x / block_width;
  //    auto block_y = y / block_height;
  //    if (block_x != _read_cache_x || block_y != _read_cache_y)
  //    {
  //      read_pixel_block(block_x * block_width, block_y * block_height,
  //                       _read_cache);
  //      _read_cache_x = block_x;
  //      _read_cache_y = block_y;
  //    }
  //    converter(pixel, _read_cache[x % block_width][y % block_height]);
  //  }

  /// Writes a compressed pixel block.
  /// @param block_x
  ///   The n-th block on the x-axis.
  /// @param block_y
  ///   The n-th block on the y-axis.
  void write_pixel_block(std::uint32_t x, std::uint32_t y,
                         std::uint32_t pixel_mask,
                         const uncompressed_pixel_block_t& pixels)
  {
    BOOST_ASSERT(x % block_width == 0);
    BOOST_ASSERT(y % block_height == 0);

    pixel_block_t pixel_block;
    squish::compress_masked(&pixels[0][0], pixel_mask, pixel_block.data(),
                            _flags);
    std::memcpy(data(x / block_width, y / block_height), pixel_block.data(),
                pixel_t::size_in_bytes);
  }

private:
  storage_t* _buffer;
  std::uint32_t _width;
  std::uint32_t _height;
  squish::flags_t _flags{};
  std::uint32_t _read_cache_x = std::numeric_limits<std::uint32_t>::max();
  std::uint32_t _read_cache_y = std::numeric_limits<std::uint32_t>::max();
  uncompressed_pixel_block_t _read_cache;
};
}

#endif
