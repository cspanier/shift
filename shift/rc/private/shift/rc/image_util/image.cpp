#include "shift/rc/image_util/image.hpp"
#include "shift/rc/image_util/linear_image_view.hpp"
#include "shift/rc/image_util/block_image_view.hpp"
#include "shift/rc/image_util/convert.hpp"
#include <boost/assert.hpp>

namespace shift::rc::image_util
{
template <resource_db::image_format Format, typename Pixel>
using format_pair =
  core::pair<core::value_to_type<resource_db::image_format, Format>, Pixel>;

/// A compile-time map which associates enumerants from image_format with a
/// pixel definition type.
using format_map = core::map<
  format_pair<resource_db::image_format::r8_unorm, pixel_r8_unorm>,
  format_pair<resource_db::image_format::r8_snorm, pixel_r8_snorm>,
  format_pair<resource_db::image_format::r8_srgb, pixel_r8_srgb>,
  format_pair<resource_db::image_format::r16_unorm, pixel_r16_unorm>,
  format_pair<resource_db::image_format::r16_snorm, pixel_r16_snorm>,
  format_pair<resource_db::image_format::r16_sfloat, pixel_r16_sfloat>,
  format_pair<resource_db::image_format::r32_sfloat, pixel_r32_sfloat>,
  format_pair<resource_db::image_format::r8g8_unorm, pixel_r8g8_unorm>,
  format_pair<resource_db::image_format::r8g8_snorm, pixel_r8g8_snorm>,
  format_pair<resource_db::image_format::r8g8_srgb, pixel_r8g8_srgb>,
  format_pair<resource_db::image_format::r16g16_unorm, pixel_r16g16_unorm>,
  format_pair<resource_db::image_format::r16g16_snorm, pixel_r16g16_snorm>,
  format_pair<resource_db::image_format::r16g16_sfloat, pixel_r16g16_sfloat>,
  format_pair<resource_db::image_format::r32g32_sfloat, pixel_r32g32_sfloat>,
  format_pair<resource_db::image_format::r8g8b8_unorm, pixel_r8g8b8_unorm>,
  format_pair<resource_db::image_format::r8g8b8_snorm, pixel_r8g8b8_snorm>,
  format_pair<resource_db::image_format::r8g8b8_srgb, pixel_r8g8b8_srgb>,
  format_pair<resource_db::image_format::r16g16b16_unorm,
              pixel_r16g16b16_unorm>,
  format_pair<resource_db::image_format::r16g16b16_snorm,
              pixel_r16g16b16_snorm>,
  format_pair<resource_db::image_format::r16g16b16_sfloat,
              pixel_r16g16b16_sfloat>,
  format_pair<resource_db::image_format::r32g32b32_sfloat,
              pixel_r32g32b32_sfloat>,
  format_pair<resource_db::image_format::b8g8r8_unorm, pixel_b8g8r8_unorm>,
  format_pair<resource_db::image_format::b8g8r8_snorm, pixel_b8g8r8_snorm>,
  format_pair<resource_db::image_format::b8g8r8_srgb, pixel_b8g8r8_srgb>,
  format_pair<resource_db::image_format::r8g8b8a8_unorm, pixel_r8g8b8a8_unorm>,
  format_pair<resource_db::image_format::r8g8b8a8_snorm, pixel_r8g8b8a8_snorm>,
  format_pair<resource_db::image_format::r8g8b8a8_srgb, pixel_r8g8b8a8_srgb>,
  format_pair<resource_db::image_format::r16g16b16a16_unorm,
              pixel_r16g16b16a16_unorm>,
  format_pair<resource_db::image_format::r16g16b16a16_snorm,
              pixel_r16g16b16a16_snorm>,
  format_pair<resource_db::image_format::r16g16b16a16_sfloat,
              pixel_r16g16b16a16_sfloat>,
  format_pair<resource_db::image_format::r32g32b32a32_sfloat,
              pixel_r32g32b32a32_sfloat>,
  format_pair<resource_db::image_format::b8g8r8a8_unorm, pixel_b8g8r8a8_unorm>,
  format_pair<resource_db::image_format::b8g8r8a8_snorm, pixel_b8g8r8a8_snorm>,
  format_pair<resource_db::image_format::b8g8r8a8_srgb, pixel_b8g8r8a8_srgb>,
  format_pair<resource_db::image_format::a8b8g8r8_unorm, pixel_a8b8g8r8_unorm>,
  format_pair<resource_db::image_format::a8b8g8r8_snorm, pixel_a8b8g8r8_snorm>,
  format_pair<resource_db::image_format::bc1_rgb_unorm_block,
              pixel_bc1_rgb_unorm>,
  format_pair<resource_db::image_format::bc1_rgb_srgb_block,
              pixel_bc1_rgb_srgb>,
  format_pair<resource_db::image_format::bc1_rgba_unorm_block,
              pixel_bc1_rgba_unorm>,
  format_pair<resource_db::image_format::bc1_rgba_srgb_block,
              pixel_bc1_rgba_srgb>,
  format_pair<resource_db::image_format::bc2_unorm_block, pixel_bc2_unorm>,
  format_pair<resource_db::image_format::bc2_srgb_block, pixel_bc2_srgb>,
  format_pair<resource_db::image_format::bc3_unorm_block, pixel_bc3_unorm>,
  format_pair<resource_db::image_format::bc3_srgb_block, pixel_bc3_srgb>,
  format_pair<resource_db::image_format::bc4_unorm_block, pixel_bc4_unorm>,
  format_pair<resource_db::image_format::bc4_snorm_block, pixel_bc4_snorm>,
  format_pair<resource_db::image_format::bc5_unorm_block, pixel_bc5_unorm>,
  format_pair<resource_db::image_format::bc5_snorm_block, pixel_bc5_snorm>,
  format_pair<resource_db::image_format::bc6h_ufloat_block, pixel_bc6h_ufloat>,
  format_pair<resource_db::image_format::bc6h_sfloat_block, pixel_bc6h_sfloat>,
  format_pair<resource_db::image_format::bc7_unorm_block, pixel_bc7_unorm>,
  format_pair<resource_db::image_format::bc7_srgb_block, pixel_bc7_srgb>>;

using reverse_format_map = core::reverse_map_t<format_map>;

/// This function calculates the minimum required amount of memory in number of
/// bytes that an image of given dimensions and format.
template <typename Pixel>
std::size_t required_image_buffer_size(const image_descriptor& image)
{
  BOOST_ASSERT(
    (core::find_t<reverse_format_map, Pixel>::value == image.format));

  if constexpr (Pixel::is_block_format)
  {
    // Number of blocks times block size.
    constexpr auto block_width = Pixel::block_width;
    constexpr auto block_height = Pixel::block_height;
    return ((image.width + block_width - 1) / block_width) *
           ((image.height + block_height - 1) / block_height) *
           Pixel::size_in_bytes;
  }
  else
  {
    if (image.row_stride > 0)
    {
      BOOST_ASSERT(image.width * Pixel::size_in_bytes <= image.row_stride);
      return image.row_stride * image.height;
    }
    else
      return image.width * image.height * Pixel::size_in_bytes;
  }
}

struct required_image_buffer_size_dispatcher
{
  template <resource_db::image_format Format, typename Pixel>
  void operator()(const format_pair<Format, Pixel>*,
                  const image_descriptor& image, std::size_t& result) const
  {
    if (Format == image.format)
      result = required_image_buffer_size<Pixel>(image);
  }
};

std::size_t required_image_buffer_size(const image_descriptor& image)
{
  std::size_t result = 0;
  core::for_each<format_map>(required_image_buffer_size_dispatcher{}, image,
                             result);
  return result;
}

template <typename DestinationPixel, typename SourcePixel>
std::error_code convert_image(
  const destination_image_descriptor& destination_image,
  const source_image_descriptor& source_image, const convert_region& region)
{
  auto destination_image_buffer_size =
    required_image_buffer_size<DestinationPixel>(destination_image);
  if (destination_image.buffer_size < destination_image_buffer_size)
    return error_code::destination_buffer_size;
  if (DestinationPixel::is_block_format && destination_image.row_stride != 0)
  {
    return error_code::destination_row_stride_with_bc;
  }
  if (region.destination_x % DestinationPixel::block_width != 0 ||
      region.destination_y % DestinationPixel::block_height != 0 ||
      region.width % DestinationPixel::block_width != 0 ||
      region.height % DestinationPixel::block_height != 0)
  {
    return error_code::destination_region_not_block_aligned;
  }

  auto source_image_buffer_size =
    required_image_buffer_size<SourcePixel>(source_image);
  if (source_image.buffer_size < source_image_buffer_size)
    return error_code::source_buffer_size;
  if (SourcePixel::is_block_format && source_image.row_stride != 0)
    return error_code::source_row_stride_with_bc;
  if (region.source_x % SourcePixel::block_width != 0 ||
      region.source_y % SourcePixel::block_height != 0 ||
      region.width % SourcePixel::block_width != 0 ||
      region.height % SourcePixel::block_height != 0)
  {
    return error_code::source_region_not_block_aligned;
  }

  if ((std::is_same_v<DestinationPixel, SourcePixel> &&
       !DestinationPixel::is_block_format) &&
      (region.source_x == 0 && region.source_y == 0 &&
       region.width == source_image.width &&
       region.height == source_image.height) &&
      (region.destination_x == 0 && region.destination_y == 0 &&
       region.width == destination_image.width &&
       region.height == destination_image.height))
  {
    // This fast path simply copies the entire image.
    std::memcpy(destination_image.buffer, source_image.buffer,
                source_image_buffer_size);
  }
  else
  {
    using source_view_t =
      std::conditional_t<SourcePixel::is_block_format,
                         block_image_view<const SourcePixel>,
                         linear_image_view<const SourcePixel>>;
    source_view_t source_view(source_image.buffer, source_image.buffer_size,
                              source_image.width, source_image.height,
                              source_image.row_stride);
    if constexpr (DestinationPixel::is_block_format)
    {
      using destination_view_t = block_image_view<DestinationPixel>;
      destination_view_t destination_view(
        destination_image.buffer, destination_image.buffer_size,
        destination_image.width, destination_image.height, 0);

      constexpr auto block_width = DestinationPixel::block_width;
      constexpr auto block_height = DestinationPixel::block_height;

      for (std::uint32_t block_y = 0; block_y < region.height;
           block_y += block_height)
      {
        for (std::uint32_t block_x = 0; block_x < region.width;
             block_x += block_width)
        {
          // Block compression formats only work on a limited set of input
          // formats. Thus, convert the input region to an intermediate buffer.
          typename destination_view_t::uncompressed_pixel_block_t
            intermediate_pixels{};
          pixel_converter<typename destination_view_t::uncompressed_pixel_t,
                          SourcePixel>
            converter;

          // Pixels outside the image need to be masked out.
          std::uint32_t pixel_mask = 0;
          for (std::uint32_t y = 0; y < block_height; ++y)
          {
            auto absolute_y = block_y + y;
            if (absolute_y < source_image.height)
            {
              for (std::uint32_t x = 0; x < block_width; ++x)
              {
                auto absolute_x = block_x + x;
                if (absolute_x < source_image.width)
                {
                  typename source_view_t::pixel_t source_pixel;

                  source_view.read_pixel(region.source_x + x,
                                         region.source_y + y, source_pixel);
                  converter(intermediate_pixels[y * block_width + x],
                            source_pixel);

                  pixel_mask |= 1 << (y * block_width + x);
                }
              }
            }
          }
          // We must have at least one pixel.
          BOOST_ASSERT(pixel_mask != 0);
          // Compress and write the intermediate pixel block.
          destination_view.write_pixel_block(block_x, block_y, pixel_mask,
                                             intermediate_pixels);
        }
      }
    }
    else
    {
      using destination_view_t = linear_image_view<DestinationPixel>;
      destination_view_t destination_view(
        destination_image.buffer, destination_image.buffer_size,
        destination_image.width, destination_image.height,
        destination_image.row_stride);
      pixel_converter<DestinationPixel, SourcePixel> converter;

      for (std::uint32_t y = 0; y < region.height; ++y)
      {
        for (std::uint32_t x = 0; x < region.width; ++x)
        {
          typename source_view_t::pixel_t source_pixel;
          typename destination_view_t::pixel_t destination_pixel;

          source_view.read_pixel(region.source_x + x, region.source_y + y,
                                 source_pixel);
          converter(destination_pixel, source_pixel);
          destination_view.write_pixel(region.destination_x + x,
                                       region.destination_y + y,
                                       destination_pixel);
        }
      }
    }
  }
  return {};
}

/// Helper type to select the correct compile-time template instantiation of the
/// image conversion function above from a run-time source image format
/// variable.
template <typename DestinationPixel>
struct convert_image_dispatcher_2
{
  template <resource_db::image_format Format, typename SourcePixel>
  void operator()(const format_pair<Format, SourcePixel>*,
                  const destination_image_descriptor& destination_image,
                  const source_image_descriptor& source_image,
                  const convert_region& region, std::error_code& result) const
  {
    if (Format == source_image.format)
    {
      result = convert_image<DestinationPixel, SourcePixel>(
        destination_image, source_image, region);
    }
  }
};

/// Helper type to select the correct compile-time template instantiation of the
/// image conversion function above from a run-time destination image format
/// variable.
struct convert_image_dispatcher_1
{
  template <resource_db::image_format Format, typename DestinationPixel>
  void operator()(const format_pair<Format, DestinationPixel>*,
                  const destination_image_descriptor& destination_image,
                  const source_image_descriptor& source_image,
                  const convert_region& region, std::error_code& result) const
  {
    if (Format == destination_image.format)
    {
      core::for_each<format_map>(convert_image_dispatcher_2<DestinationPixel>{},
                                 destination_image, source_image, region,
                                 result);
    }
  }
};

std::error_code convert_image(
  const destination_image_descriptor& destination_image,
  const source_image_descriptor& source_image, const convert_region& region,
  convert_flags /*flags*/)
{
  /// ToDo: Make use of flags.
  if (region.width == 0 || region.height == 0)
  {
    // Nothing to do.
    return {};
  }

  if (destination_image.buffer == nullptr)
    return error_code::destination_buffer_null;
  if (source_image.buffer == nullptr)
    return error_code::source_buffer_null;
  if (destination_image.width != source_image.width ||
      destination_image.height != source_image.height)
  {
    return error_code::different_image_size;
  }

  /// ToDo: check for empty regions.
  if (region.destination_x + region.width > destination_image.width ||
      region.destination_y + region.height > destination_image.height)
  {
    return error_code::destination_region_bounds;
  }
  if (region.source_x + region.width > source_image.width ||
      region.source_y + region.height > source_image.height)
  {
    return error_code::source_region_bounds;
  }

  auto result = make_error_code(error_code::unsupported_conversion_formats);
  // This effectively translates to a huge nested switch statement for each
  // possible combination of source and destination image formats to select an
  // optimized conversion function.
  core::for_each<format_map>(convert_image_dispatcher_1{}, destination_image,
                             source_image, region, result);
  return result;
}

std::error_code resize_image(
  const destination_image_descriptor& destination_image,
  const source_image_descriptor& source_image, const resize_region& region,
  resize_flags /*flags*/)
{
  if (region.destination_width == 0 || region.destination_height == 0)
  {
    // Nothing to do.
    return {};
  }

  if (resource_db::is_block_compressed(destination_image.format) ||
      !resource_db::is_linear(destination_image.format))
  {
    return error_code::destination_format_unsupported;
  }
  if (destination_image.format != source_image.format)
    return error_code::different_image_format;
  if (destination_image.buffer == nullptr)
    return error_code::destination_buffer_null;
  if (source_image.buffer == nullptr)
    return error_code::source_buffer_null;

  if (region.destination_x + region.destination_width >
        destination_image.width ||
      region.destination_y + region.destination_height >
        destination_image.height)
  {
    return error_code::destination_region_bounds;
  }
  if (region.source_x + region.source_width > source_image.width ||
      region.source_y + region.source_height > source_image.height)
  {
    return error_code::source_region_bounds;
  }
  if (region.source_width == 0 || region.source_height == 0)
    return error_code::source_region_empty;

  /// ToDo: implement...
  return error_code::not_implemented;
}
}
