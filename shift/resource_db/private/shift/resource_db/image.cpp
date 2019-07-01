#include "shift/resource_db/image.hpp"
#include <shift/serialization2/all.hpp>
#include <shift/crypto/sha256.hpp>

namespace shift::resource_db
{
image_format to_image_format(const std::string& format_name)
{
  if (format_name == "r8_unorm")
    return image_format::r8_unorm;
  else if (format_name == "r8_snorm")
    return image_format::r8_snorm;
  else if (format_name == "r8_srgb")
    return image_format::r8_srgb;
  else if (format_name == "r16_unorm")
    return image_format::r16_unorm;
  else if (format_name == "r16_snorm")
    return image_format::r16_snorm;
  else if (format_name == "r16_sfloat")
    return image_format::r16_sfloat;
  else if (format_name == "r32_sfloat")
    return image_format::r32_sfloat;
  else if (format_name == "r8g8_unorm")
    return image_format::r8g8_unorm;
  else if (format_name == "r8g8_snorm")
    return image_format::r8g8_snorm;
  else if (format_name == "r8g8_srgb")
    return image_format::r8g8_srgb;
  else if (format_name == "r16g16_unorm")
    return image_format::r16g16_unorm;
  else if (format_name == "r16g16_snorm")
    return image_format::r16g16_snorm;
  else if (format_name == "r16g16_sfloat")
    return image_format::r16g16_sfloat;
  else if (format_name == "r32g32_sfloat")
    return image_format::r32g32_sfloat;
  else if (format_name == "r8g8b8_unorm")
    return image_format::r8g8b8_unorm;
  else if (format_name == "r8g8b8_snorm")
    return image_format::r8g8b8_snorm;
  else if (format_name == "r8g8b8_srgb")
    return image_format::r8g8b8_srgb;
  else if (format_name == "r16g16b16_unorm")
    return image_format::r16g16b16_unorm;
  else if (format_name == "r16g16b16_snorm")
    return image_format::r16g16b16_snorm;
  else if (format_name == "r16g16b16_sfloat")
    return image_format::r16g16b16_sfloat;
  else if (format_name == "r32g32b32_sfloat")
    return image_format::r32g32b32_sfloat;
  else if (format_name == "b8g8r8_unorm")
    return image_format::b8g8r8_unorm;
  else if (format_name == "b8g8r8_snorm")
    return image_format::b8g8r8_snorm;
  else if (format_name == "b8g8r8_srgb")
    return image_format::b8g8r8_srgb;
  else if (format_name == "r8g8b8a8_unorm")
    return image_format::r8g8b8a8_unorm;
  else if (format_name == "r8g8b8a8_snorm")
    return image_format::r8g8b8a8_snorm;
  else if (format_name == "r8g8b8a8_srgb")
    return image_format::r8g8b8a8_srgb;
  else if (format_name == "r16g16b16a16_unorm")
    return image_format::r16g16b16a16_unorm;
  else if (format_name == "r16g16b16a16_snorm")
    return image_format::r16g16b16a16_snorm;
  else if (format_name == "r16g16b16a16_sfloat")
    return image_format::r16g16b16a16_sfloat;
  else if (format_name == "r32g32b32a32_sfloat")
    return image_format::r32g32b32a32_sfloat;
  else if (format_name == "b8g8r8a8_unorm")
    return image_format::b8g8r8a8_unorm;
  else if (format_name == "b8g8r8a8_snorm")
    return image_format::b8g8r8a8_snorm;
  else if (format_name == "b8g8r8a8_srgb")
    return image_format::b8g8r8a8_srgb;
  else if (format_name == "a8b8g8r8_unorm")
    return image_format::a8b8g8r8_unorm;
  else if (format_name == "a8b8g8r8_snorm")
    return image_format::a8b8g8r8_snorm;
  else if (format_name == "bc1_rgb_unorm")
    return image_format::bc1_rgb_unorm_block;
  else if (format_name == "bc1_rgb_srgb")
    return image_format::bc1_rgb_srgb_block;
  else if (format_name == "bc1_rgba_unorm")
    return image_format::bc1_rgba_unorm_block;
  else if (format_name == "bc1_rgba_srgb")
    return image_format::bc1_rgba_srgb_block;
  else if (format_name == "bc2_unorm")
    return image_format::bc2_unorm_block;
  else if (format_name == "bc2_srgb")
    return image_format::bc2_srgb_block;
  else if (format_name == "bc3_unorm")
    return image_format::bc3_unorm_block;
  else if (format_name == "bc3_srgb")
    return image_format::bc3_srgb_block;
  else if (format_name == "bc4_unorm")
    return image_format::bc4_unorm_block;
  else if (format_name == "bc4_snorm")
    return image_format::bc4_snorm_block;
  else if (format_name == "bc5_unorm")
    return image_format::bc5_unorm_block;
  else if (format_name == "bc5_snorm")
    return image_format::bc5_snorm_block;
  else if (format_name == "bc6h_ufloat")
    return image_format::bc6h_ufloat_block;
  else if (format_name == "bc6h_sfloat")
    return image_format::bc6h_sfloat_block;
  else if (format_name == "bc7_unorm")
    return image_format::bc7_unorm_block;
  else if (format_name == "bc7_srgb")
    return image_format::bc7_srgb_block;
  else
    return image_format::undefined;
}

///
serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, mipmap_info& mipmap)
{
  archive >> mipmap.buffer >> mipmap.offset >> mipmap.width >> mipmap.height >>
    mipmap.depth;
  mipmap.width = std::max(mipmap.width, 1u);
  mipmap.height = std::max(mipmap.height, 1u);
  mipmap.depth = std::max(mipmap.depth, 1u);
  return archive;
}

///
serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive, const mipmap_info& mipmap)
{
  archive << mipmap.buffer << mipmap.offset << mipmap.width << mipmap.height
          << mipmap.depth;
  return archive;
}

void image::load(resource_id id, boost::iostreams::filtering_istream& stream)
{
  _id = id;
  serialization2::compact_input_archive<> archive{stream};
  archive >> format >> array_element_count >> face_count >> mipmaps;
}

void image::save(boost::iostreams::filtering_ostream& stream) const
{
  serialization2::compact_output_archive<> archive{stream};
  archive << format << array_element_count << face_count << mipmaps;
}

void image::hash(crypto::sha256& context) const
{
  context << "image" << format << array_element_count << face_count;
  for (const auto& mipmap : mipmaps)
  {
    context << mipmap.buffer.id() << mipmap.offset << mipmap.width
            << mipmap.height << mipmap.depth;
  }
}
}
