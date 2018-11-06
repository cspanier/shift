#ifndef SHIFT_RESOURCE_IMAGE_HPP
#define SHIFT_RESOURCE_IMAGE_HPP

#include <cstdint>
#include <array>
#include <vector>
#include <boost/gil/typedefs.hpp>
#include "shift/resource/resource_ptr.hpp"
#include "shift/resource/buffer.hpp"

namespace shift::resource
{
enum class image_format : std::uint8_t
{
  undefined,

  r8_unorm,
  r8_snorm,
  // r8_uint,
  // r8_sint,
  r8_srgb,
  r16_unorm,
  r16_snorm,
  // r16_uint,
  // r16_sint,
  // r16_sfloat,
  // r32_uint,
  // r32_sint,
  r32_sfloat,

  r8g8_unorm,
  r8g8_snorm,
  // r8g8_uint,
  // r8g8_sint,
  r8g8_srgb,
  r16g16_unorm,
  r16g16_snorm,
  // r16g16_uint,
  // r16g16_sint,
  r16g16_sfloat,
  // r32g32_uint,
  // r32g32_sint,
  r32g32_sfloat,

  r8g8b8_unorm,
  r8g8b8_snorm,
  // r8g8b8_uint,
  // r8g8b8_sint,
  r8g8b8_srgb,
  r16g16b16_unorm,
  r16g16b16_snorm,
  // r16g16b16_uint,
  // r16g16b16_sint,
  r16g16b16_sfloat,
  // r32g32b32_uint,
  // r32g32b32_sint,
  r32g32b32_sfloat,

  b8g8r8_unorm,
  b8g8r8_snorm,
  // b8g8r8_uint,
  // b8g8r8_sint,
  b8g8r8_srgb,

  r8g8b8a8_unorm,
  r8g8b8a8_snorm,
  // r8g8b8a8_uint,
  // r8g8b8a8_sint,
  r8g8b8a8_srgb,
  r16g16b16a16_unorm,
  r16g16b16a16_snorm,
  // r16g16b16a16_uint,
  // r16g16b16a16_sint,
  r16g16b16a16_sfloat,
  // r32g32b32a32_uint,
  // r32g32b32a32_sint,
  r32g32b32a32_sfloat,

  b8g8r8a8_unorm,
  b8g8r8a8_snorm,
  // b8g8r8a8_uint,
  // b8g8r8a8_sint,
  b8g8r8a8_srgb,

  a8b8g8r8_unorm,
  a8b8g8r8_snorm,
  // a8b8g8r8_uint,
  // a8b8g8r8_sint,
  // a8b8g8r8_srgb
};

template <typename GilPixel, bool SRGB>
struct format_from_gil_pixel;

template <>
struct format_from_gil_pixel<boost::gil::gray8_pixel_t, false>
{
  static constexpr image_format value = image_format::r8_unorm;
};

template <>
struct format_from_gil_pixel<boost::gil::gray8s_pixel_t, false>
{
  static constexpr image_format value = image_format::r8_snorm;
};

template <>
struct format_from_gil_pixel<boost::gil::gray8_pixel_t, true>
{
  static constexpr image_format value = image_format::r8_srgb;
};

template <>
struct format_from_gil_pixel<boost::gil::gray16_pixel_t, false>
{
  static constexpr image_format value = image_format::r16_unorm;
};

template <>
struct format_from_gil_pixel<boost::gil::gray16s_pixel_t, false>
{
  static constexpr image_format value = image_format::r16_snorm;
};

template <>
struct format_from_gil_pixel<boost::gil::gray32f_pixel_t, false>
{
  static constexpr image_format value = image_format::r32_sfloat;
};

template <>
struct format_from_gil_pixel<boost::gil::rgb8_pixel_t, false>
{
  static constexpr image_format value = image_format::r8g8b8_unorm;
};

template <>
struct format_from_gil_pixel<boost::gil::rgb8s_pixel_t, false>
{
  static constexpr image_format value = image_format::r8g8b8_snorm;
};

template <>
struct format_from_gil_pixel<boost::gil::rgb8_pixel_t, true>
{
  static constexpr image_format value = image_format::r8g8b8_srgb;
};

template <>
struct format_from_gil_pixel<boost::gil::rgb16_pixel_t, false>
{
  static constexpr image_format value = image_format::r16g16b16_unorm;
};

template <>
struct format_from_gil_pixel<boost::gil::rgb16s_pixel_t, false>
{
  static constexpr image_format value = image_format::r16g16b16_snorm;
};

template <>
struct format_from_gil_pixel<boost::gil::rgb32f_pixel_t, false>
{
  static constexpr image_format value = image_format::r32g32b32_sfloat;
};

template <>
struct format_from_gil_pixel<boost::gil::bgr8_pixel_t, false>
{
  static constexpr image_format value = image_format::b8g8r8_unorm;
};

template <>
struct format_from_gil_pixel<boost::gil::bgr8s_pixel_t, false>
{
  static constexpr image_format value = image_format::b8g8r8_snorm;
};

template <>
struct format_from_gil_pixel<boost::gil::bgr8_pixel_t, true>
{
  static constexpr image_format value = image_format::b8g8r8_srgb;
};

template <>
struct format_from_gil_pixel<boost::gil::rgba8_pixel_t, false>
{
  static constexpr image_format value = image_format::r8g8b8a8_unorm;
};

template <>
struct format_from_gil_pixel<boost::gil::rgba8s_pixel_t, false>
{
  static constexpr image_format value = image_format::r8g8b8a8_snorm;
};

template <>
struct format_from_gil_pixel<boost::gil::rgba8_pixel_t, true>
{
  static constexpr image_format value = image_format::r8g8b8a8_srgb;
};

template <>
struct format_from_gil_pixel<boost::gil::rgba16_pixel_t, false>
{
  static constexpr image_format value = image_format::r16g16b16a16_unorm;
};

template <>
struct format_from_gil_pixel<boost::gil::rgba16s_pixel_t, false>
{
  static constexpr image_format value = image_format::r16g16b16a16_snorm;
};

template <>
struct format_from_gil_pixel<boost::gil::rgba32f_pixel_t, false>
{
  static constexpr image_format value = image_format::r32g32b32a32_sfloat;
};

template <>
struct format_from_gil_pixel<boost::gil::bgra8_pixel_t, false>
{
  static constexpr image_format value = image_format::b8g8r8a8_unorm;
};

template <>
struct format_from_gil_pixel<boost::gil::bgra8s_pixel_t, false>
{
  static constexpr image_format value = image_format::b8g8r8a8_snorm;
};

template <>
struct format_from_gil_pixel<boost::gil::bgra8_pixel_t, true>
{
  static constexpr image_format value = image_format::b8g8r8a8_srgb;
};

template <>
struct format_from_gil_pixel<boost::gil::abgr8_pixel_t, false>
{
  static constexpr image_format value = image_format::a8b8g8r8_unorm;
};

template <>
struct format_from_gil_pixel<boost::gil::abgr8s_pixel_t, false>
{
  static constexpr image_format value = image_format::a8b8g8r8_snorm;
};

///
struct mipmap_info
{
  resource_ptr<resource::buffer> buffer;
  std::uint32_t offset;
  std::uint32_t width;
  std::uint32_t height;
  std::uint32_t depth;

  std::uint32_t num_pixels()
  {
    return width * height * depth;
  }
};

///
class image final : public resource_base
{
public:
  static constexpr resource_type static_type = resource_type::image;

  /// Default constructor.
  image() noexcept : resource_base(static_type)
  {
  }

  /// @see resource_base::load.
  void load(resource_id id, boost::iostreams::filtering_istream& stream) final;

  /// @see resource_base::save.
  void save(boost::iostreams::filtering_ostream& stream) const final;

public:
  image_format format = image_format::undefined;
  std::uint32_t array_element_count = 0;
  std::uint32_t face_count = 0;
  std::vector<mipmap_info> mipmaps;

protected:
  /// @see resource_base::hash.
  void hash(crypto::sha256& context) const final;
};
}

#endif
