#include "shift/render/vk/utility.hpp"
#include "shift/render/vk/application.hpp"
#include "shift/render/vk/renderer_impl.hpp"
#include "shift/render/vk/renderer.hpp"
#include "shift/render/vk/layer1/buffer.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include <shift/core/mpl.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/gil.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::render::vk
{
std::uint32_t pixel_size(vk::format format)
{
  switch (format)
  {
  case vk::format::r4_g4_unorm_pack8:
    return 1;

  case vk::format::r4_g4_b4_a4_unorm_pack16:
  case vk::format::b4_g4_r4_a4_unorm_pack16:
  case vk::format::r5_g6_b5_unorm_pack16:
  case vk::format::b5_g6_r5_unorm_pack16:
  case vk::format::r5_g5_b5_a1_unorm_pack16:
  case vk::format::b5_g5_r5_a1_unorm_pack16:
  case vk::format::a1_r5_g5_b5_unorm_pack16:
    return 2;

  case vk::format::r8_unorm:
  case vk::format::r8_snorm:
  case vk::format::r8_uscaled:
  case vk::format::r8_sscaled:
  case vk::format::r8_uint:
  case vk::format::r8_sint:
  case vk::format::r8_srgb:
    return 1;

  case vk::format::r8_g8_unorm:
  case vk::format::r8_g8_snorm:
  case vk::format::r8_g8_uscaled:
  case vk::format::r8_g8_sscaled:
  case vk::format::r8_g8_uint:
  case vk::format::r8_g8_sint:
  case vk::format::r8_g8_srgb:
    return 2;

  case vk::format::r8_g8_b8_unorm:
  case vk::format::r8_g8_b8_snorm:
  case vk::format::r8_g8_b8_uscaled:
  case vk::format::r8_g8_b8_sscaled:
  case vk::format::r8_g8_b8_uint:
  case vk::format::r8_g8_b8_sint:
  case vk::format::r8_g8_b8_srgb:
  case vk::format::b8_g8_r8_unorm:
  case vk::format::b8_g8_r8_snorm:
  case vk::format::b8_g8_r8_uscaled:
  case vk::format::b8_g8_r8_sscaled:
  case vk::format::b8_g8_r8_uint:
  case vk::format::b8_g8_r8_sint:
  case vk::format::b8_g8_r8_srgb:
    return 3;

  case vk::format::r8_g8_b8_a8_unorm:
  case vk::format::r8_g8_b8_a8_snorm:
  case vk::format::r8_g8_b8_a8_uscaled:
  case vk::format::r8_g8_b8_a8_sscaled:
  case vk::format::r8_g8_b8_a8_uint:
  case vk::format::r8_g8_b8_a8_sint:
  case vk::format::r8_g8_b8_a8_srgb:
  case vk::format::b8_g8_r8_a8_unorm:
  case vk::format::b8_g8_r8_a8_snorm:
  case vk::format::b8_g8_r8_a8_uscaled:
  case vk::format::b8_g8_r8_a8_sscaled:
  case vk::format::b8_g8_r8_a8_uint:
  case vk::format::b8_g8_r8_a8_sint:
  case vk::format::b8_g8_r8_a8_srgb:
  case vk::format::a8_b8_g8_r8_unorm_pack32:
  case vk::format::a8_b8_g8_r8_snorm_pack32:
  case vk::format::a8_b8_g8_r8_uscaled_pack32:
  case vk::format::a8_b8_g8_r8_sscaled_pack32:
  case vk::format::a8_b8_g8_r8_uint_pack32:
  case vk::format::a8_b8_g8_r8_sint_pack32:
  case vk::format::a8_b8_g8_r8_srgb_pack32:
  case vk::format::a2_r10_g10_b10_unorm_pack32:
  case vk::format::a2_r10_g10_b10_snorm_pack32:
  case vk::format::a2_r10_g10_b10_uscaled_pack32:
  case vk::format::a2_r10_g10_b10_sscaled_pack32:
  case vk::format::a2_r10_g10_b10_uint_pack32:
  case vk::format::a2_r10_g10_b10_sint_pack32:
  case vk::format::a2_b10_g10_r10_unorm_pack32:
  case vk::format::a2_b10_g10_r10_snorm_pack32:
  case vk::format::a2_b10_g10_r10_uscaled_pack32:
  case vk::format::a2_b10_g10_r10_sscaled_pack32:
  case vk::format::a2_b10_g10_r10_uint_pack32:
  case vk::format::a2_b10_g10_r10_sint_pack32:
    return 4;

  case vk::format::r16_unorm:
  case vk::format::r16_snorm:
  case vk::format::r16_uscaled:
  case vk::format::r16_sscaled:
  case vk::format::r16_uint:
  case vk::format::r16_sint:
  case vk::format::r16_sfloat:
    return 2;

  case vk::format::r16_g16_unorm:
  case vk::format::r16_g16_snorm:
  case vk::format::r16_g16_uscaled:
  case vk::format::r16_g16_sscaled:
  case vk::format::r16_g16_uint:
  case vk::format::r16_g16_sint:
  case vk::format::r16_g16_sfloat:
    return 4;

  case vk::format::r16_g16_b16_unorm:
  case vk::format::r16_g16_b16_snorm:
  case vk::format::r16_g16_b16_uscaled:
  case vk::format::r16_g16_b16_sscaled:
  case vk::format::r16_g16_b16_uint:
  case vk::format::r16_g16_b16_sint:
  case vk::format::r16_g16_b16_sfloat:
    return 6;

  case vk::format::r16_g16_b16_a16_unorm:
  case vk::format::r16_g16_b16_a16_snorm:
  case vk::format::r16_g16_b16_a16_uscaled:
  case vk::format::r16_g16_b16_a16_sscaled:
  case vk::format::r16_g16_b16_a16_uint:
  case vk::format::r16_g16_b16_a16_sint:
  case vk::format::r16_g16_b16_a16_sfloat:
    return 8;

  case vk::format::r32_uint:
  case vk::format::r32_sint:
  case vk::format::r32_sfloat:
    return 4;

  case vk::format::r32_g32_uint:
  case vk::format::r32_g32_sint:
  case vk::format::r32_g32_sfloat:
    return 8;

  case vk::format::r32_g32_b32_uint:
  case vk::format::r32_g32_b32_sint:
  case vk::format::r32_g32_b32_sfloat:
    return 12;

  case vk::format::r32_g32_b32_a32_uint:
  case vk::format::r32_g32_b32_a32_sint:
  case vk::format::r32_g32_b32_a32_sfloat:
    return 16;

  case vk::format::r64_uint:
  case vk::format::r64_sint:
  case vk::format::r64_sfloat:
    return 8;

  case vk::format::r64_g64_uint:
  case vk::format::r64_g64_sint:
  case vk::format::r64_g64_sfloat:
    return 16;

  case vk::format::r64_g64_b64_uint:
  case vk::format::r64_g64_b64_sint:
  case vk::format::r64_g64_b64_sfloat:
    return 24;

  case vk::format::r64_g64_b64_a64_uint:
  case vk::format::r64_g64_b64_a64_sint:
  case vk::format::r64_g64_b64_a64_sfloat:
    return 32;

  case vk::format::b10_g11_r11_ufloat_pack32:
  case vk::format::e5_b9_g9_r9_ufloat_pack32:
    return 4;

  case vk::format::d16_unorm:
    return 2;

  case vk::format::x8_d24_unorm_pack32:
  case vk::format::d32_sfloat:
    return 4;

  case vk::format::s8_uint:
    return 1;

  case vk::format::d16_unorm_s8_uint:
    return 3;

  case vk::format::d24_unorm_s8_uint:
    return 4;

  case vk::format::d32_sfloat_s8_uint:
    return 5;

  default:
    // A debug assert should be enough as all internal formats are static.
    BOOST_ASSERT(false);
    return 0;
  }
}

vk::format format_from_resource(resource_db::image_format format)
{
  switch (format)
  {
  case resource_db::image_format::r8_unorm:
    return vk::format::r8_unorm;
  case resource_db::image_format::r8_snorm:
    return vk::format::r8_snorm;
  case resource_db::image_format::r8_srgb:
    return vk::format::r8_srgb;
  case resource_db::image_format::r16_unorm:
    return vk::format::r16_unorm;
  case resource_db::image_format::r16_snorm:
    return vk::format::r16_snorm;
  case resource_db::image_format::r32_sfloat:
    return vk::format::r32_sfloat;

  case resource_db::image_format::r8g8_unorm:
    return vk::format::r8_g8_unorm;
  case resource_db::image_format::r8g8_snorm:
    return vk::format::r8_g8_snorm;
  case resource_db::image_format::r8g8_srgb:
    return vk::format::r8_g8_srgb;
  case resource_db::image_format::r16g16_unorm:
    return vk::format::r16_g16_unorm;
  case resource_db::image_format::r16g16_snorm:
    return vk::format::r16_g16_snorm;
  case resource_db::image_format::r16g16_sfloat:
    return vk::format::r16_g16_sfloat;
  case resource_db::image_format::r32g32_sfloat:
    return vk::format::r32_g32_sfloat;

  case resource_db::image_format::r8g8b8_unorm:
    return vk::format::r8_g8_b8_unorm;
  case resource_db::image_format::r8g8b8_snorm:
    return vk::format::r8_g8_b8_snorm;
  case resource_db::image_format::r8g8b8_srgb:
    return vk::format::r8_g8_b8_srgb;
  case resource_db::image_format::r16g16b16_unorm:
    return vk::format::r16_g16_b16_unorm;
  case resource_db::image_format::r16g16b16_snorm:
    return vk::format::r16_g16_b16_snorm;
  case resource_db::image_format::r16g16b16_sfloat:
    return vk::format::r16_g16_b16_sfloat;
  case resource_db::image_format::r32g32b32_sfloat:
    return vk::format::r32_g32_b32_sfloat;

  case resource_db::image_format::b8g8r8_unorm:
    return vk::format::b8_g8_r8_unorm;
  case resource_db::image_format::b8g8r8_snorm:
    return vk::format::b8_g8_r8_snorm;
  case resource_db::image_format::b8g8r8_srgb:
    return vk::format::b8_g8_r8_srgb;

  case resource_db::image_format::r8g8b8a8_unorm:
    return vk::format::r8_g8_b8_a8_unorm;
  case resource_db::image_format::r8g8b8a8_snorm:
    return vk::format::r8_g8_b8_a8_snorm;
  case resource_db::image_format::r8g8b8a8_srgb:
    return vk::format::r8_g8_b8_a8_srgb;
  case resource_db::image_format::r16g16b16a16_unorm:
    return vk::format::r16_g16_b16_a16_unorm;
  case resource_db::image_format::r16g16b16a16_snorm:
    return vk::format::r16_g16_b16_a16_snorm;
  case resource_db::image_format::r16g16b16a16_sfloat:
    return vk::format::r16_g16_b16_a16_sfloat;
  case resource_db::image_format::r32g32b32a32_sfloat:
    return vk::format::r32_g32_b32_a32_sfloat;

  case resource_db::image_format::b8g8r8a8_unorm:
    return vk::format::b8_g8_r8_a8_unorm;
  case resource_db::image_format::b8g8r8a8_snorm:
    return vk::format::b8_g8_r8_a8_snorm;
  case resource_db::image_format::b8g8r8a8_srgb:
    return vk::format::b8_g8_r8_a8_srgb;

  default:
    BOOST_ASSERT(false);
    return vk::format::undefined;
  }
}

vk::format format_from_resource(
  resource_db::vertex_attribute_data_type data_type,
  resource_db::vertex_attribute_component_type component_type)
{
  switch (data_type)
  {
  case resource_db::vertex_attribute_data_type::scalar:
    switch (component_type)
    {
    case resource_db::vertex_attribute_component_type::int8:
      return vk::format::r8_sint;
    case resource_db::vertex_attribute_component_type::uint8:
      return vk::format::r8_uint;
    case resource_db::vertex_attribute_component_type::int16:
      return vk::format::r16_sint;
    case resource_db::vertex_attribute_component_type::uint16:
      return vk::format::r16_uint;
    case resource_db::vertex_attribute_component_type::int32:
      return vk::format::r32_sint;
    case resource_db::vertex_attribute_component_type::uint32:
      return vk::format::r32_uint;
    case resource_db::vertex_attribute_component_type::float32:
      return vk::format::r32_sfloat;

    default:
      BOOST_ASSERT(false);
      return vk::format::undefined;
    }

  case resource_db::vertex_attribute_data_type::vec2:
    switch (component_type)
    {
    case resource_db::vertex_attribute_component_type::int8:
      return vk::format::r8_g8_sint;
    case resource_db::vertex_attribute_component_type::uint8:
      return vk::format::r8_g8_uint;
    case resource_db::vertex_attribute_component_type::int16:
      return vk::format::r16_g16_sint;
    case resource_db::vertex_attribute_component_type::uint16:
      return vk::format::r16_g16_uint;
    case resource_db::vertex_attribute_component_type::int32:
      return vk::format::r32_g32_sint;
    case resource_db::vertex_attribute_component_type::uint32:
      return vk::format::r32_g32_uint;
    case resource_db::vertex_attribute_component_type::float32:
      return vk::format::r32_g32_sfloat;

    default:
      BOOST_ASSERT(false);
      return vk::format::undefined;
    }

  case resource_db::vertex_attribute_data_type::vec3:
    switch (component_type)
    {
    case resource_db::vertex_attribute_component_type::int8:
      return vk::format::r8_g8_b8_sint;
    case resource_db::vertex_attribute_component_type::uint8:
      return vk::format::r8_g8_b8_uint;
    case resource_db::vertex_attribute_component_type::int16:
      return vk::format::r16_g16_b16_sint;
    case resource_db::vertex_attribute_component_type::uint16:
      return vk::format::r16_g16_b16_uint;
    case resource_db::vertex_attribute_component_type::int32:
      return vk::format::r32_g32_b32_sint;
    case resource_db::vertex_attribute_component_type::uint32:
      return vk::format::r32_g32_b32_uint;
    case resource_db::vertex_attribute_component_type::float32:
      return vk::format::r32_g32_b32_sfloat;

    default:
      BOOST_ASSERT(false);
      return vk::format::undefined;
    }

  case resource_db::vertex_attribute_data_type::vec4:
    switch (component_type)
    {
    case resource_db::vertex_attribute_component_type::int8:
      return vk::format::r8_g8_b8_a8_sint;
    case resource_db::vertex_attribute_component_type::uint8:
      return vk::format::r8_g8_b8_a8_uint;
    case resource_db::vertex_attribute_component_type::int16:
      return vk::format::r16_g16_b16_a16_sint;
    case resource_db::vertex_attribute_component_type::uint16:
      return vk::format::r16_g16_b16_a16_uint;
    case resource_db::vertex_attribute_component_type::int32:
      return vk::format::r32_g32_b32_a32_sint;
    case resource_db::vertex_attribute_component_type::uint32:
      return vk::format::r32_g32_b32_a32_uint;
    case resource_db::vertex_attribute_component_type::float32:
      return vk::format::r32_g32_b32_a32_sfloat;

    default:
      BOOST_ASSERT(false);
      return vk::format::undefined;
    }

  default:
    BOOST_ASSERT(false);
    return vk::format::undefined;
  }
}

vk::format device_image_format(vk::format format)
{
  switch (format)
  {
  case vk::format::r8_g8_b8_unorm:
    return vk::format::r8_g8_b8_a8_unorm;
  case vk::format::r8_g8_b8_snorm:
    return vk::format::r8_g8_b8_a8_snorm;
  case vk::format::r8_g8_b8_srgb:
    return vk::format::r8_g8_b8_a8_srgb;
  default:
    return format;
  }
}

namespace detail
{
  template <vk::format Key, typename T>
  struct format_type_pair
  {
  };

  using format_to_gil_image_map = core::set<
    format_type_pair<vk::format::r8_unorm, boost::gil::gray8_image_t>,
    format_type_pair<vk::format::r8_snorm, boost::gil::gray8s_image_t>,
    format_type_pair<vk::format::r8_srgb, boost::gil::gray8_image_t>,
    format_type_pair<vk::format::r16_unorm, boost::gil::gray16_image_t>,
    format_type_pair<vk::format::r16_snorm, boost::gil::gray16s_image_t>,
    format_type_pair<vk::format::r32_sfloat, boost::gil::gray32f_image_t>,
    format_type_pair<vk::format::r8_g8_b8_unorm, boost::gil::rgb8_image_t>,
    format_type_pair<vk::format::r8_g8_b8_snorm, boost::gil::rgb8s_image_t>,
    format_type_pair<vk::format::r8_g8_b8_srgb, boost::gil::rgb8_image_t>,
    format_type_pair<vk::format::r16_g16_b16_unorm, boost::gil::rgb16_image_t>,
    format_type_pair<vk::format::r16_g16_b16_snorm, boost::gil::rgb16s_image_t>,
    format_type_pair<vk::format::r32_g32_b32_sfloat,
                     boost::gil::rgb32f_image_t>,
    format_type_pair<vk::format::b8_g8_r8_unorm, boost::gil::bgr8_image_t>,
    format_type_pair<vk::format::b8_g8_r8_snorm, boost::gil::bgr8s_image_t>,
    format_type_pair<vk::format::b8_g8_r8_srgb, boost::gil::bgr8_image_t>,
    format_type_pair<vk::format::r8_g8_b8_a8_unorm, boost::gil::rgba8_image_t>,
    format_type_pair<vk::format::r8_g8_b8_a8_snorm, boost::gil::rgba8s_image_t>,
    format_type_pair<vk::format::r8_g8_b8_a8_srgb, boost::gil::rgba8_image_t>,
    format_type_pair<vk::format::r16_g16_b16_a16_unorm,
                     boost::gil::rgba16_image_t>,
    format_type_pair<vk::format::r16_g16_b16_a16_snorm,
                     boost::gil::rgba16s_image_t>,
    format_type_pair<vk::format::r32_g32_b32_a32_sfloat,
                     boost::gil::rgba32f_image_t>,
    format_type_pair<vk::format::b8_g8_r8_a8_unorm, boost::gil::bgra8_image_t>,
    format_type_pair<vk::format::b8_g8_r8_a8_snorm, boost::gil::bgra8s_image_t>,
    format_type_pair<vk::format::b8_g8_r8_a8_srgb, boost::gil::bgra8_image_t>>;

  ///
  template <typename SourceGilImage, typename DestinationGilImage>
  struct copy_or_convert_image_to_buffer
  {
    void operator()(resource_db::image& source, vk::layer1::buffer& target,
                    std::uint32_t mip_level_begin,
                    std::uint32_t mip_level_end) const
    {
      namespace gil = boost::gil;

      using source_pixel_t = typename SourceGilImage::value_type;
      using destination_pixel_t = typename DestinationGilImage::value_type;

      std::size_t offset = 0;
      for (auto mip_level = mip_level_begin; mip_level < mip_level_end;
           ++mip_level)
      {
        auto& mipmap = source.mipmaps[mip_level];
        gil::copy_and_convert_pixels(
          gil::interleaved_view(mipmap.width, mipmap.height,
                                reinterpret_cast<source_pixel_t*>(
                                  &mipmap.buffer->storage[mipmap.offset]),
                                mipmap.width * sizeof(source_pixel_t)),
          gil::interleaved_view(
            mipmap.width, mipmap.height,
            reinterpret_cast<destination_pixel_t*>(target.address(offset)),
            mipmap.width * sizeof(destination_pixel_t)));
        offset += mipmap.width * mipmap.height * 4;
      }
    }
  };

  ///
  template <typename SourceAndDestinationGilImage>
  struct copy_or_convert_image_to_buffer<SourceAndDestinationGilImage,
                                         SourceAndDestinationGilImage>
  {
    void operator()(resource_db::image& source, vk::layer1::buffer& target,
                    std::uint32_t mip_level_begin,
                    std::uint32_t mip_level_end) const
    {
      std::size_t offset = 0;
      resource_db::buffer* last_buffer = nullptr;
      for (auto mip_level = mip_level_begin; mip_level < mip_level_end;
           ++mip_level)
      {
        auto& mipmap = source.mipmaps[mip_level];
        if (&*mipmap.buffer == last_buffer)
          continue;
        std::memcpy(target.address(offset), mipmap.buffer->storage.data(),
                    mipmap.buffer->storage.size());
        offset += mipmap.buffer->storage.size();
        last_buffer = &*mipmap.buffer;
      }
    }
  };

  /// A visitor type used in two nested for_each loops to select a source
  /// and destination Boost GIL image type at compile time.
  struct copy_image_to_buffer_visitor
  {
    /// This method gets called for each source format.
    template <vk::format SourceFormat, typename SourceGilImage>
    void operator()(
      const format_type_pair<SourceFormat, SourceGilImage>* source_dummy,
      vk::format source_format, resource_db::image& source,
      vk::format destination_format, vk::layer1::buffer& buffer,
      std::uint32_t mip_level_begin, std::uint32_t mip_level_end)
    {
      if (SourceFormat == source_format)
      {
        core::for_each<format_to_gil_image_map>(*this, source_dummy, source,
                                                destination_format, buffer,
                                                mip_level_begin, mip_level_end);
      }
    }

    /// This method gets called for each destination format.
    template <vk::format SourceFormat, typename SourceGilImage,
              vk::format DestinationFormat, typename DestinationGilImage>
    void operator()(
      const format_type_pair<DestinationFormat, DestinationGilImage>*,
      const format_type_pair<SourceFormat, SourceGilImage>*,
      resource_db::image& source, vk::format destination_format,
      vk::layer1::buffer& buffer, std::uint32_t mip_level_begin,
      std::uint32_t mip_level_end)
    {
      if (DestinationFormat == destination_format)
      {
        copy_or_convert_image_to_buffer<SourceGilImage, DestinationGilImage>()(
          source, buffer, mip_level_begin, mip_level_end);
      }
    }
  };
}

vk::shared_ptr<vk::layer1::buffer> copy_image_to_staging_buffer(
  vk::layer1::device& device, resource_db::image& source,
  vk::format destination_format)
{
  namespace gil = boost::gil;

  auto source_format = format_from_resource(source.format);
  if (destination_format == vk::format::undefined)
    destination_format = source_format;

  // Calculate size of memory needed for staging buffer.
  auto destination_pixel_size = pixel_size(destination_format);
  std::size_t staging_buffer_size = 0;
  for (const auto& mipmap : source.mipmaps)
  {
    staging_buffer_size +=
      mipmap.width * mipmap.height * mipmap.depth * destination_pixel_size;
  }

  auto staging_buffer = vk::make_framed_shared<vk::layer1::buffer>(
    device, vk::buffer_create_info(
              /* next */ nullptr,
              /* flags */ vk::buffer_create_flag::none,
              /* size */ staging_buffer_size,
              /* usage */ vk::buffer_usage_flag::transfer_src_bit,
              /* sharing_mode */ vk::sharing_mode::exclusive,
              /* queue_family_index_count */ 0,
              /* queue_family_indices */ nullptr));
  staging_buffer->allocate_storage(memory_pool::staging_buffers);
  staging_buffer->bind_storage();

  detail::copy_image_to_buffer_visitor visitor;
  core::for_each<detail::format_to_gil_image_map>(
    visitor, source_format, source, destination_format, *staging_buffer, 0u,
    static_cast<std::uint32_t>(source.mipmaps.size()));

  return staging_buffer;
}

void copy_image_to_staging_buffer(resource_db::image& source,
                                  vk::layer1::buffer& staging_buffer,
                                  vk::format destination_format,
                                  std::uint32_t mip_level_begin,
                                  std::uint32_t mip_level_end)
{
  auto source_format = format_from_resource(source.format);

  [[maybe_unused]] auto mip_level_count =
    static_cast<std::uint32_t>(source.mipmaps.size());
  BOOST_ASSERT(mip_level_begin < mip_level_count);
  BOOST_ASSERT(mip_level_end <= mip_level_count);

  detail::copy_image_to_buffer_visitor visitor;
  core::for_each<detail::format_to_gil_image_map>(
    visitor, source_format, source, destination_format, staging_buffer,
    mip_level_begin, mip_level_end);
}
}
