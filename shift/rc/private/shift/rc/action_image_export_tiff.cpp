#include "shift/rc/action_image_export_tiff.hpp"
#include "shift/rc/resource_compiler_impl.hpp"
#include <shift/rc/image_util/tiff_io.hpp>
#include <shift/rc/image_util/image.hpp>
#include <shift/resource_db/image.hpp>
#include <shift/log/log.hpp>
#include <shift/math/utility.hpp>
#include <shift/math/vector.hpp>
#include <shift/core/mpl.hpp>
#include <shift/core/string_util.hpp>
#include <filesystem>
#include <gsl/gsl>
//#include <image/image.h>
#include <tiffio.h>

namespace shift::rc
{
namespace fs = std::filesystem;

std::uint16_t samples_per_pixel(resource_db::image_format format)
{
  switch (format)
  {
  case resource_db::image_format::r8_unorm:
  case resource_db::image_format::r8_snorm:
  case resource_db::image_format::r8_srgb:
  case resource_db::image_format::r16_unorm:
  case resource_db::image_format::r16_snorm:
  case resource_db::image_format::r16_sfloat:
  case resource_db::image_format::r32_sfloat:
  case resource_db::image_format::bc4_unorm_block:
  case resource_db::image_format::bc4_snorm_block:
    return 1;

  case resource_db::image_format::r8g8_unorm:
  case resource_db::image_format::r8g8_snorm:
  case resource_db::image_format::r8g8_srgb:
  case resource_db::image_format::r16g16_unorm:
  case resource_db::image_format::r16g16_snorm:
  case resource_db::image_format::r16g16_sfloat:
  case resource_db::image_format::r32g32_sfloat:
  case resource_db::image_format::r8g8b8_unorm:
  case resource_db::image_format::r8g8b8_snorm:
  case resource_db::image_format::r8g8b8_srgb:
  case resource_db::image_format::bc5_unorm_block:
  case resource_db::image_format::bc5_snorm_block:
    return 2;

  case resource_db::image_format::r16g16b16_unorm:
  case resource_db::image_format::r16g16b16_snorm:
  case resource_db::image_format::r16g16b16_sfloat:
  case resource_db::image_format::r32g32b32_sfloat:
  case resource_db::image_format::b8g8r8_unorm:
  case resource_db::image_format::b8g8r8_snorm:
  case resource_db::image_format::b8g8r8_srgb:
  case resource_db::image_format::bc1_rgb_unorm_block:
  case resource_db::image_format::bc1_rgb_srgb_block:
    return 3;

  case resource_db::image_format::r8g8b8a8_unorm:
  case resource_db::image_format::r8g8b8a8_snorm:
  case resource_db::image_format::r8g8b8a8_srgb:
  case resource_db::image_format::r16g16b16a16_unorm:
  case resource_db::image_format::r16g16b16a16_snorm:
  case resource_db::image_format::r16g16b16a16_sfloat:
  case resource_db::image_format::r32g32b32a32_sfloat:
  case resource_db::image_format::b8g8r8a8_unorm:
  case resource_db::image_format::b8g8r8a8_snorm:
  case resource_db::image_format::b8g8r8a8_srgb:
  case resource_db::image_format::a8b8g8r8_unorm:
  case resource_db::image_format::a8b8g8r8_snorm:
  case resource_db::image_format::bc1_rgba_unorm_block:
  case resource_db::image_format::bc1_rgba_srgb_block:
  case resource_db::image_format::bc2_unorm_block:
  case resource_db::image_format::bc2_srgb_block:
  case resource_db::image_format::bc3_unorm_block:
  case resource_db::image_format::bc3_srgb_block:
  case resource_db::image_format::bc6h_ufloat_block:
  case resource_db::image_format::bc6h_sfloat_block:
  case resource_db::image_format::bc7_unorm_block:
  case resource_db::image_format::bc7_srgb_block:
    return 4;

  default:
    return 0;
  }
}

std::uint16_t bits_per_sample(resource_db::image_format format)
{
  switch (format)
  {
  case resource_db::image_format::r8_unorm:
  case resource_db::image_format::r8_snorm:
  case resource_db::image_format::r8_srgb:
  case resource_db::image_format::r8g8_unorm:
  case resource_db::image_format::r8g8_snorm:
  case resource_db::image_format::r8g8_srgb:
  case resource_db::image_format::r8g8b8_unorm:
  case resource_db::image_format::r8g8b8_snorm:
  case resource_db::image_format::r8g8b8_srgb:
  case resource_db::image_format::b8g8r8_unorm:
  case resource_db::image_format::b8g8r8_snorm:
  case resource_db::image_format::b8g8r8_srgb:
  case resource_db::image_format::r8g8b8a8_unorm:
  case resource_db::image_format::r8g8b8a8_snorm:
  case resource_db::image_format::r8g8b8a8_srgb:
  case resource_db::image_format::b8g8r8a8_unorm:
  case resource_db::image_format::b8g8r8a8_snorm:
  case resource_db::image_format::b8g8r8a8_srgb:
  case resource_db::image_format::a8b8g8r8_unorm:
  case resource_db::image_format::a8b8g8r8_snorm:
  case resource_db::image_format::bc1_rgb_unorm_block:
  case resource_db::image_format::bc1_rgb_srgb_block:
  case resource_db::image_format::bc1_rgba_unorm_block:
  case resource_db::image_format::bc1_rgba_srgb_block:
  case resource_db::image_format::bc2_unorm_block:
  case resource_db::image_format::bc2_srgb_block:
  case resource_db::image_format::bc3_unorm_block:
  case resource_db::image_format::bc3_srgb_block:
  case resource_db::image_format::bc4_unorm_block:
  case resource_db::image_format::bc4_snorm_block:
  case resource_db::image_format::bc5_unorm_block:
  case resource_db::image_format::bc5_snorm_block:
  case resource_db::image_format::bc7_unorm_block:
  case resource_db::image_format::bc7_srgb_block:
    return 8;

  case resource_db::image_format::r16_unorm:
  case resource_db::image_format::r16_snorm:
  case resource_db::image_format::r16_sfloat:
  case resource_db::image_format::r16g16_unorm:
  case resource_db::image_format::r16g16_snorm:
  case resource_db::image_format::r16g16_sfloat:
  case resource_db::image_format::r16g16b16_unorm:
  case resource_db::image_format::r16g16b16_snorm:
  case resource_db::image_format::r16g16b16_sfloat:
  case resource_db::image_format::r16g16b16a16_unorm:
  case resource_db::image_format::r16g16b16a16_snorm:
  case resource_db::image_format::r16g16b16a16_sfloat:
  case resource_db::image_format::bc6h_ufloat_block:
  case resource_db::image_format::bc6h_sfloat_block:
    return 16;

  case resource_db::image_format::r32_sfloat:
  case resource_db::image_format::r32g32_sfloat:
  case resource_db::image_format::r32g32b32_sfloat:
  case resource_db::image_format::r32g32b32a32_sfloat:
    return 32;

  default:
    return 0;
  }
}

image_util::tiff_samples_format samples_format(resource_db::image_format format)
{
  switch (format)
  {
  case resource_db::image_format::r8_unorm:
  case resource_db::image_format::r16_unorm:
  case resource_db::image_format::r8g8_unorm:
  case resource_db::image_format::r16g16_unorm:
  case resource_db::image_format::r8g8b8_unorm:
  case resource_db::image_format::r16g16b16_unorm:
  case resource_db::image_format::b8g8r8_unorm:
  case resource_db::image_format::r8g8b8a8_unorm:
  case resource_db::image_format::r16g16b16a16_unorm:
  case resource_db::image_format::b8g8r8a8_unorm:
  case resource_db::image_format::a8b8g8r8_unorm:
  case resource_db::image_format::bc1_rgb_unorm_block:
  case resource_db::image_format::bc1_rgba_unorm_block:
  case resource_db::image_format::bc2_unorm_block:
  case resource_db::image_format::bc3_unorm_block:
  case resource_db::image_format::bc4_unorm_block:
  case resource_db::image_format::bc5_unorm_block:
  case resource_db::image_format::bc7_unorm_block:
    return image_util::tiff_samples_format::unsigned_int;

  case resource_db::image_format::r8_srgb:
  case resource_db::image_format::r8g8_srgb:
  case resource_db::image_format::r8g8b8_srgb:
  case resource_db::image_format::b8g8r8_srgb:
  case resource_db::image_format::r8g8b8a8_srgb:
  case resource_db::image_format::b8g8r8a8_srgb:
  case resource_db::image_format::bc1_rgb_srgb_block:
  case resource_db::image_format::bc1_rgba_srgb_block:
  case resource_db::image_format::bc2_srgb_block:
  case resource_db::image_format::bc3_srgb_block:
  case resource_db::image_format::bc7_srgb_block:
    return image_util::tiff_samples_format::unsigned_int;

  case resource_db::image_format::r8_snorm:
  case resource_db::image_format::r16_snorm:
  case resource_db::image_format::r8g8_snorm:
  case resource_db::image_format::r16g16_snorm:
  case resource_db::image_format::r8g8b8_snorm:
  case resource_db::image_format::r16g16b16_snorm:
  case resource_db::image_format::b8g8r8_snorm:
  case resource_db::image_format::r8g8b8a8_snorm:
  case resource_db::image_format::r16g16b16a16_snorm:
  case resource_db::image_format::b8g8r8a8_snorm:
  case resource_db::image_format::a8b8g8r8_snorm:
  case resource_db::image_format::bc4_snorm_block:
  case resource_db::image_format::bc5_snorm_block:
    return image_util::tiff_samples_format::signed_int;

  case resource_db::image_format::r16_sfloat:
  case resource_db::image_format::r32_sfloat:
  case resource_db::image_format::r16g16_sfloat:
  case resource_db::image_format::r32g32_sfloat:
  case resource_db::image_format::r16g16b16_sfloat:
  case resource_db::image_format::r32g32b32_sfloat:
  case resource_db::image_format::r16g16b16a16_sfloat:
  case resource_db::image_format::r32g32b32a32_sfloat:
  case resource_db::image_format::bc6h_ufloat_block:
  case resource_db::image_format::bc6h_sfloat_block:
    return image_util::tiff_samples_format::floating_point;

  default:
    return image_util::tiff_samples_format::other;
  }
}

image_util::tiff_icc_profile_category icc_profile_category(
  resource_db::image_format format)
{
  switch (format)
  {
  case resource_db::image_format::r8_unorm:
  case resource_db::image_format::r8_snorm:
  case resource_db::image_format::r16_unorm:
  case resource_db::image_format::r16_snorm:
  case resource_db::image_format::r16_sfloat:
  case resource_db::image_format::r32_sfloat:
  case resource_db::image_format::r8g8_unorm:
  case resource_db::image_format::r8g8_snorm:
  case resource_db::image_format::r16g16_unorm:
  case resource_db::image_format::r16g16_snorm:
  case resource_db::image_format::r16g16_sfloat:
  case resource_db::image_format::r32g32_sfloat:
  case resource_db::image_format::r8g8b8_unorm:
  case resource_db::image_format::r8g8b8_snorm:
  case resource_db::image_format::r16g16b16_unorm:
  case resource_db::image_format::r16g16b16_snorm:
  case resource_db::image_format::r16g16b16_sfloat:
  case resource_db::image_format::r32g32b32_sfloat:
  case resource_db::image_format::b8g8r8_unorm:
  case resource_db::image_format::b8g8r8_snorm:
  case resource_db::image_format::r8g8b8a8_unorm:
  case resource_db::image_format::r8g8b8a8_snorm:
  case resource_db::image_format::r16g16b16a16_unorm:
  case resource_db::image_format::r16g16b16a16_snorm:
  case resource_db::image_format::r16g16b16a16_sfloat:
  case resource_db::image_format::r32g32b32a32_sfloat:
  case resource_db::image_format::b8g8r8a8_unorm:
  case resource_db::image_format::b8g8r8a8_snorm:
  case resource_db::image_format::a8b8g8r8_unorm:
  case resource_db::image_format::a8b8g8r8_snorm:
  case resource_db::image_format::bc1_rgb_unorm_block:
  case resource_db::image_format::bc1_rgba_unorm_block:
  case resource_db::image_format::bc2_unorm_block:
  case resource_db::image_format::bc3_unorm_block:
  case resource_db::image_format::bc4_unorm_block:
  case resource_db::image_format::bc4_snorm_block:
  case resource_db::image_format::bc5_unorm_block:
  case resource_db::image_format::bc5_snorm_block:
  case resource_db::image_format::bc6h_ufloat_block:
  case resource_db::image_format::bc6h_sfloat_block:
  case resource_db::image_format::bc7_unorm_block:
    return image_util::tiff_icc_profile_category::linear;

  case resource_db::image_format::r8_srgb:
  case resource_db::image_format::r8g8_srgb:
  case resource_db::image_format::r8g8b8_srgb:
  case resource_db::image_format::b8g8r8_srgb:
  case resource_db::image_format::r8g8b8a8_srgb:
  case resource_db::image_format::b8g8r8a8_srgb:
  case resource_db::image_format::bc1_rgb_srgb_block:
  case resource_db::image_format::bc1_rgba_srgb_block:
  case resource_db::image_format::bc2_srgb_block:
  case resource_db::image_format::bc3_srgb_block:
  case resource_db::image_format::bc7_srgb_block:
    return image_util::tiff_icc_profile_category::srgb;

  default:
    return image_util::tiff_icc_profile_category::custom;
  }
}

action_image_export_tiff::action_image_export_tiff()
: action_base(action_name, action_version)
{
}

bool action_image_export_tiff::process(resource_compiler_impl& compiler,
                                       job_description& job) const
{
  using namespace std::string_literals;

  auto& io = image_util::tiff_io::singleton_instance();

  if (job.inputs.size() != 1)
  {
    log::error() << "The " << action_image_export_tiff::action_name
                 << " action can only process one input at a time.";
    return false;
  }
  const auto& input = *job.inputs.begin()->second;

  if (!fs::exists(input.file->path) || !fs::is_regular_file(input.file->path))
  {
    log::error() << "Cannot find input file " << input.file->path << ".";
    return false;
  }

  BOOST_ASSERT(job.rule);

  auto target_format = resource_db::to_image_format(
    parser::json::has(job.rule->options, "target-format")
      ? parser::json::get<std::string>(job.rule->options.at("target-format"))
      : "auto"s);
  if (target_format == resource_db::image_format::undefined)
    target_format = resource_db::image_format::r8g8b8a8_srgb;

  auto& repository = resource_db::repository::singleton_instance();
  auto input_image = repository.load<resource_db::image>(input.file->path);
  if (!input_image)
  {
    log::error() << "Cannot load image " << input.file->path << ".";
    return false;
  }
  if (input_image->array_element_count > 1)
  {
    log::error()
      << "The image-export-tiff action does not yet support texture arrays.";
    return false;
  }
  if (input_image->face_count > 1)
  {
    log::error() << "The image-export-tiff action does not yet support "
                    "texture with more than one face.";
    return false;
  }
  if (input_image->mipmaps.empty())
  {
    log::error()
      << "The image-export-tiff action does not support empty textures.";
    return false;
  }

  auto& input_map = input_image->mipmaps.front();
  auto& input_buffer = *input_map.buffer;
  if (input_map.depth != 1)
  {
    log::error()
      << "The image-export-tiff action does not yet support volume textures.";
    return false;
  }
  if (input_buffer.storage.size() < input_map.offset)
    return false;

  image_util::source_image_descriptor source;
  source.width = input_map.width;
  source.height = input_map.height;
  source.row_stride = 0;
  source.format = input_image->format;
  source.buffer_size = input_buffer.storage.size() - input_map.offset;
  source.buffer = input_buffer.storage.data() + input_map.offset;

  std::vector<std::byte> target_buffer;
  image_util::destination_image_descriptor destination;
  destination.width = source.width;
  destination.height = source.height;
  destination.row_stride = 0;
  destination.format = target_format;
  target_buffer.resize(image_util::required_image_buffer_size(destination));
  destination.buffer_size = target_buffer.size();
  destination.buffer = target_buffer.data();

  image_util::convert_region region;
  region.destination_x = 0;
  region.destination_y = 0;
  region.source_x = 0;
  region.source_y = 0;
  region.width = source.width;
  region.height = source.height;

  if (auto error = image_util::convert_image(destination, source, region, {});
      error)
  {
    log::error() << "Image compression failed: " << error;
    return false;
  }

  std::vector<image_util::tiff_image> output_images;
  auto& output_image = output_images.emplace_back();
  output_image.samples_per_pixel = samples_per_pixel(target_format);
  output_image.extra_samples = 0;
  output_image.bits_per_sample = bits_per_sample(target_format);
  output_image.samples_format = samples_format(target_format);
  output_image.compression = image_util::tiff_compression::zstd;
  output_image.width = input_map.width;
  output_image.height = input_map.height;
  output_image.rows_per_strip = input_map.height;
  output_image.pixel_data = std::move(target_buffer);
  output_image.icc_profile_category = icc_profile_category(target_format);

  if (auto filename = job.output_file_path("image", {});
      io.save(filename, output_images, false))
  {
    input.file->alias = compiler.push(filename, job);
  }
  else
  {
    log::error() << "Failed saving image " << filename << ".";
    return false;
  }

  return true;
}
}
