#ifndef SHIFT_RESOURCE_DB_IMAGE_HPP
#define SHIFT_RESOURCE_DB_IMAGE_HPP

#include <cstdint>
#include <array>
#include <vector>
#include "shift/resource_db/types.hpp"
#include "shift/resource_db/resource_ptr.hpp"
#include "shift/resource_db/buffer.hpp"

namespace shift::resource_db
{
template <>
struct resource_traits<image>
{
  static constexpr resource_type type_id = resource_type::image;
};

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
  r16_sfloat,
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
  // a8b8g8r8_srgb,

  // S3TC Compressed Texture Image Formats
  bc1_rgb_unorm_block,
  bc1_rgb_srgb_block,
  bc1_rgba_unorm_block,
  bc1_rgba_srgb_block,
  bc2_unorm_block,
  bc2_srgb_block,
  bc3_unorm_block,
  bc3_srgb_block,

  // RGTC Compressed Texture Image Formats
  bc4_unorm_block,
  bc4_snorm_block,
  bc5_unorm_block,
  bc5_snorm_block,

  // BPTC Compressed Texture Image Formats
  bc6h_ufloat_block,
  bc6h_sfloat_block,
  bc7_unorm_block,
  bc7_srgb_block
};

inline constexpr bool is_block_compressed(image_format format)
{
  switch (format)
  {
  case image_format::bc1_rgb_unorm_block:
  case image_format::bc1_rgb_srgb_block:
  case image_format::bc1_rgba_unorm_block:
  case image_format::bc1_rgba_srgb_block:
  case image_format::bc2_unorm_block:
  case image_format::bc2_srgb_block:
  case image_format::bc3_unorm_block:
  case image_format::bc3_srgb_block:
  case image_format::bc4_unorm_block:
  case image_format::bc4_snorm_block:
  case image_format::bc5_unorm_block:
  case image_format::bc5_snorm_block:
  case image_format::bc6h_ufloat_block:
  case image_format::bc6h_sfloat_block:
  case image_format::bc7_unorm_block:
  case image_format::bc7_srgb_block:
    return true;

  case image_format::undefined:
  case image_format::r8_unorm:
  case image_format::r8_snorm:
  // case image_format::r8_uint:
  // case image_format::r8_sint:
  case image_format::r8_srgb:
  case image_format::r16_unorm:
  case image_format::r16_snorm:
  // case image_format::r16_uint:
  // case image_format::r16_sint:
  case image_format::r16_sfloat:
  // case image_format::r32_uint:
  // case image_format::r32_sint:
  case image_format::r32_sfloat:
  case image_format::r8g8_unorm:
  case image_format::r8g8_snorm:
  // case image_format::r8g8_uint:
  // case image_format::r8g8_sint:
  case image_format::r8g8_srgb:
  case image_format::r16g16_unorm:
  case image_format::r16g16_snorm:
  // case image_format::r16g16_uint:
  // case image_format::r16g16_sint:
  case image_format::r16g16_sfloat:
  // case image_format::r32g32_uint:
  // case image_format::r32g32_sint:
  case image_format::r32g32_sfloat:
  case image_format::r8g8b8_unorm:
  case image_format::r8g8b8_snorm:
  // case image_format::r8g8b8_uint:
  // case image_format::r8g8b8_sint:
  case image_format::r8g8b8_srgb:
  case image_format::r16g16b16_unorm:
  case image_format::r16g16b16_snorm:
  // case image_format::r16g16b16_uint:
  // case image_format::r16g16b16_sint:
  case image_format::r16g16b16_sfloat:
  // case image_format::r32g32b32_uint:
  // case image_format::r32g32b32_sint:
  case image_format::r32g32b32_sfloat:
  case image_format::b8g8r8_unorm:
  case image_format::b8g8r8_snorm:
  // case image_format::b8g8r8_uint:
  // case image_format::b8g8r8_sint:
  case image_format::b8g8r8_srgb:
  case image_format::r8g8b8a8_unorm:
  case image_format::r8g8b8a8_snorm:
  // case image_format::r8g8b8a8_uint:
  // case image_format::r8g8b8a8_sint:
  case image_format::r8g8b8a8_srgb:
  case image_format::r16g16b16a16_unorm:
  case image_format::r16g16b16a16_snorm:
  // case image_format::r16g16b16a16_uint:
  // case image_format::r16g16b16a16_sint:
  case image_format::r16g16b16a16_sfloat:
  // case image_format::r32g32b32a32_uint:
  // case image_format::r32g32b32a32_sint:
  case image_format::r32g32b32a32_sfloat:
  case image_format::b8g8r8a8_unorm:
  case image_format::b8g8r8a8_snorm:
  // case image_format::b8g8r8a8_uint:
  // case image_format::b8g8r8a8_sint:
  case image_format::b8g8r8a8_srgb:
  case image_format::a8b8g8r8_unorm:
  case image_format::a8b8g8r8_snorm:
    // case image_format::a8b8g8r8_uint:
    // case image_format::a8b8g8r8_sint:
    // case image_format::a8b8g8r8_srgb:
    return false;
  }
}

inline constexpr bool is_linear(image_format format)
{
  switch (format)
  {
  case image_format::r8_unorm:
  case image_format::r8_snorm:
  // case image_format::r8_uint:
  // case image_format::r8_sint:
  case image_format::r16_unorm:
  case image_format::r16_snorm:
  // case image_format::r16_uint:
  // case image_format::r16_sint:
  case image_format::r16_sfloat:
  // case image_format::r32_uint:
  // case image_format::r32_sint:
  case image_format::r32_sfloat:
  case image_format::r8g8_unorm:
  case image_format::r8g8_snorm:
  // case image_format::r8g8_uint:
  // case image_format::r8g8_sint:
  case image_format::r16g16_unorm:
  case image_format::r16g16_snorm:
  // case image_format::r16g16_uint:
  // case image_format::r16g16_sint:
  case image_format::r16g16_sfloat:
  // case image_format::r32g32_uint:
  // case image_format::r32g32_sint:
  case image_format::r32g32_sfloat:
  case image_format::r8g8b8_unorm:
  case image_format::r8g8b8_snorm:
  // case image_format::r8g8b8_uint:
  // case image_format::r8g8b8_sint:
  case image_format::r16g16b16_unorm:
  case image_format::r16g16b16_snorm:
  // case image_format::r16g16b16_uint:
  // case image_format::r16g16b16_sint:
  case image_format::r16g16b16_sfloat:
  // case image_format::r32g32b32_uint:
  // case image_format::r32g32b32_sint:
  case image_format::r32g32b32_sfloat:
  case image_format::b8g8r8_unorm:
  case image_format::b8g8r8_snorm:
  // case image_format::b8g8r8_uint:
  // case image_format::b8g8r8_sint:
  case image_format::r8g8b8a8_unorm:
  case image_format::r8g8b8a8_snorm:
  // case image_format::r8g8b8a8_uint:
  // case image_format::r8g8b8a8_sint:
  case image_format::r16g16b16a16_unorm:
  case image_format::r16g16b16a16_snorm:
  // case image_format::r16g16b16a16_uint:
  // case image_format::r16g16b16a16_sint:
  case image_format::r16g16b16a16_sfloat:
  // case image_format::r32g32b32a32_uint:
  // case image_format::r32g32b32a32_sint:
  case image_format::r32g32b32a32_sfloat:
  case image_format::b8g8r8a8_unorm:
  case image_format::b8g8r8a8_snorm:
  // case image_format::b8g8r8a8_uint:
  // case image_format::b8g8r8a8_sint:
  case image_format::a8b8g8r8_unorm:
  case image_format::a8b8g8r8_snorm:
  // case image_format::a8b8g8r8_uint:
  // case image_format::a8b8g8r8_sint:
  case image_format::bc1_rgb_unorm_block:
  case image_format::bc1_rgba_unorm_block:
  case image_format::bc2_unorm_block:
  case image_format::bc3_unorm_block:
  case image_format::bc4_unorm_block:
  case image_format::bc4_snorm_block:
  case image_format::bc5_unorm_block:
  case image_format::bc5_snorm_block:
  case image_format::bc6h_ufloat_block:
  case image_format::bc6h_sfloat_block:
  case image_format::bc7_unorm_block:
    return true;

  case image_format::undefined:
  case image_format::r8_srgb:
  case image_format::r8g8_srgb:
  case image_format::r8g8b8_srgb:
  case image_format::b8g8r8_srgb:
  case image_format::r8g8b8a8_srgb:
  case image_format::b8g8r8a8_srgb:
  // case image_format::a8b8g8r8_srgb:
  case image_format::bc1_rgb_srgb_block:
  case image_format::bc1_rgba_srgb_block:
  case image_format::bc2_srgb_block:
  case image_format::bc3_srgb_block:
  case image_format::bc7_srgb_block:
    return false;
  }
}

inline constexpr bool is_srgb(image_format format)
{
  switch (format)
  {
  case image_format::r8_srgb:
  case image_format::r8g8_srgb:
  case image_format::r8g8b8_srgb:
  case image_format::b8g8r8_srgb:
  case image_format::r8g8b8a8_srgb:
  case image_format::b8g8r8a8_srgb:
  // case image_format::a8b8g8r8_srgb:
  case image_format::bc1_rgb_srgb_block:
  case image_format::bc1_rgba_srgb_block:
  case image_format::bc2_srgb_block:
  case image_format::bc3_srgb_block:
  case image_format::bc7_srgb_block:
    return true;

  case image_format::undefined:
  case image_format::r8_unorm:
  case image_format::r8_snorm:
  // case image_format::r8_uint:
  // case image_format::r8_sint:
  case image_format::r16_unorm:
  case image_format::r16_snorm:
  // case image_format::r16_uint:
  // case image_format::r16_sint:
  case image_format::r16_sfloat:
  // case image_format::r32_uint:
  // case image_format::r32_sint:
  case image_format::r32_sfloat:
  case image_format::r8g8_unorm:
  case image_format::r8g8_snorm:
  // case image_format::r8g8_uint:
  // case image_format::r8g8_sint:
  case image_format::r16g16_unorm:
  case image_format::r16g16_snorm:
  // case image_format::r16g16_uint:
  // case image_format::r16g16_sint:
  case image_format::r16g16_sfloat:
  // case image_format::r32g32_uint:
  // case image_format::r32g32_sint:
  case image_format::r32g32_sfloat:
  case image_format::r8g8b8_unorm:
  case image_format::r8g8b8_snorm:
  // case image_format::r8g8b8_uint:
  // case image_format::r8g8b8_sint:
  case image_format::r16g16b16_unorm:
  case image_format::r16g16b16_snorm:
  // case image_format::r16g16b16_uint:
  // case image_format::r16g16b16_sint:
  case image_format::r16g16b16_sfloat:
  // case image_format::r32g32b32_uint:
  // case image_format::r32g32b32_sint:
  case image_format::r32g32b32_sfloat:
  case image_format::b8g8r8_unorm:
  case image_format::b8g8r8_snorm:
  // case image_format::b8g8r8_uint:
  // case image_format::b8g8r8_sint:
  case image_format::r8g8b8a8_unorm:
  case image_format::r8g8b8a8_snorm:
  // case image_format::r8g8b8a8_uint:
  // case image_format::r8g8b8a8_sint:
  case image_format::r16g16b16a16_unorm:
  case image_format::r16g16b16a16_snorm:
  // case image_format::r16g16b16a16_uint:
  // case image_format::r16g16b16a16_sint:
  case image_format::r16g16b16a16_sfloat:
  // case image_format::r32g32b32a32_uint:
  // case image_format::r32g32b32a32_sint:
  case image_format::r32g32b32a32_sfloat:
  case image_format::b8g8r8a8_unorm:
  case image_format::b8g8r8a8_snorm:
  // case image_format::b8g8r8a8_uint:
  // case image_format::b8g8r8a8_sint:
  case image_format::a8b8g8r8_unorm:
  case image_format::a8b8g8r8_snorm:
  // case image_format::a8b8g8r8_uint:
  // case image_format::a8b8g8r8_sint:
  case image_format::bc1_rgb_unorm_block:
  case image_format::bc1_rgba_unorm_block:
  case image_format::bc2_unorm_block:
  case image_format::bc3_unorm_block:
  case image_format::bc4_unorm_block:
  case image_format::bc4_snorm_block:
  case image_format::bc5_unorm_block:
  case image_format::bc5_snorm_block:
  case image_format::bc6h_ufloat_block:
  case image_format::bc6h_sfloat_block:
  case image_format::bc7_unorm_block:
    return false;
  }
}

///
struct mipmap_info
{
  resource_ptr<resource_db::buffer> buffer;
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
  /// Default constructor.
  image() noexcept : resource_base(resource_traits<image>::type_id)
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
