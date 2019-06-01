#include "shift/rc/action_image_import_tiff.hpp"
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
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>
#include <boost/gil/extension/numeric/resample.hpp>
#include <boost/gil.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <gsl/gsl>
// #include <image/image.h>
// #include <tiffio.h>

namespace shift::rc
{
namespace fs = std::filesystem;
namespace gil = boost::gil;

/// Convert an image format name to image_format.
/// @param format_name
///   One of the pre-defined image format names, or any other string.
/// @return
///   The image format associated with the value of format_name, or
///   resource_db::image_format::undefined if no association exists.
resource_db::image_format to_format(const std::string& format_name)
{
  if (format_name == "r8_unorm")
    return resource_db::image_format::r8_unorm;
  else if (format_name == "r8_snorm")
    return resource_db::image_format::r8_snorm;
  else if (format_name == "r8_srgb")
    return resource_db::image_format::r8_srgb;
  else if (format_name == "r16_unorm")
    return resource_db::image_format::r16_unorm;
  else if (format_name == "r16_snorm")
    return resource_db::image_format::r16_snorm;
  else if (format_name == "r16_sfloat")
    return resource_db::image_format::r16_sfloat;
  else if (format_name == "r32_sfloat")
    return resource_db::image_format::r32_sfloat;
  else if (format_name == "r8g8_unorm")
    return resource_db::image_format::r8g8_unorm;
  else if (format_name == "r8g8_snorm")
    return resource_db::image_format::r8g8_snorm;
  else if (format_name == "r8g8_srgb")
    return resource_db::image_format::r8g8_srgb;
  else if (format_name == "r16g16_unorm")
    return resource_db::image_format::r16g16_unorm;
  else if (format_name == "r16g16_snorm")
    return resource_db::image_format::r16g16_snorm;
  else if (format_name == "r16g16_sfloat")
    return resource_db::image_format::r16g16_sfloat;
  else if (format_name == "r32g32_sfloat")
    return resource_db::image_format::r32g32_sfloat;
  else if (format_name == "r8g8b8_unorm")
    return resource_db::image_format::r8g8b8_unorm;
  else if (format_name == "r8g8b8_snorm")
    return resource_db::image_format::r8g8b8_snorm;
  else if (format_name == "r8g8b8_srgb")
    return resource_db::image_format::r8g8b8_srgb;
  else if (format_name == "r16g16b16_unorm")
    return resource_db::image_format::r16g16b16_unorm;
  else if (format_name == "r16g16b16_snorm")
    return resource_db::image_format::r16g16b16_snorm;
  else if (format_name == "r16g16b16_sfloat")
    return resource_db::image_format::r16g16b16_sfloat;
  else if (format_name == "r32g32b32_sfloat")
    return resource_db::image_format::r32g32b32_sfloat;
  else if (format_name == "b8g8r8_unorm")
    return resource_db::image_format::b8g8r8_unorm;
  else if (format_name == "b8g8r8_snorm")
    return resource_db::image_format::b8g8r8_snorm;
  else if (format_name == "b8g8r8_srgb")
    return resource_db::image_format::b8g8r8_srgb;
  else if (format_name == "r8g8b8a8_unorm")
    return resource_db::image_format::r8g8b8a8_unorm;
  else if (format_name == "r8g8b8a8_snorm")
    return resource_db::image_format::r8g8b8a8_snorm;
  else if (format_name == "r8g8b8a8_srgb")
    return resource_db::image_format::r8g8b8a8_srgb;
  else if (format_name == "r16g16b16a16_unorm")
    return resource_db::image_format::r16g16b16a16_unorm;
  else if (format_name == "r16g16b16a16_snorm")
    return resource_db::image_format::r16g16b16a16_snorm;
  else if (format_name == "r16g16b16a16_sfloat")
    return resource_db::image_format::r16g16b16a16_sfloat;
  else if (format_name == "r32g32b32a32_sfloat")
    return resource_db::image_format::r32g32b32a32_sfloat;
  else if (format_name == "b8g8r8a8_unorm")
    return resource_db::image_format::b8g8r8a8_unorm;
  else if (format_name == "b8g8r8a8_snorm")
    return resource_db::image_format::b8g8r8a8_snorm;
  else if (format_name == "b8g8r8a8_srgb")
    return resource_db::image_format::b8g8r8a8_srgb;
  else if (format_name == "a8b8g8r8_unorm")
    return resource_db::image_format::a8b8g8r8_unorm;
  else if (format_name == "a8b8g8r8_snorm")
    return resource_db::image_format::a8b8g8r8_snorm;
  else if (format_name == "bc1_rgb_unorm")
    return resource_db::image_format::bc1_rgb_unorm_block;
  else if (format_name == "bc1_rgb_srgb")
    return resource_db::image_format::bc1_rgb_srgb_block;
  else if (format_name == "bc1_rgba_unorm")
    return resource_db::image_format::bc1_rgba_unorm_block;
  else if (format_name == "bc1_rgba_srgb")
    return resource_db::image_format::bc1_rgba_srgb_block;
  else if (format_name == "bc2_unorm")
    return resource_db::image_format::bc2_unorm_block;
  else if (format_name == "bc2_srgb")
    return resource_db::image_format::bc2_srgb_block;
  else if (format_name == "bc3_unorm")
    return resource_db::image_format::bc3_unorm_block;
  else if (format_name == "bc3_srgb")
    return resource_db::image_format::bc3_srgb_block;
  else if (format_name == "bc4_unorm")
    return resource_db::image_format::bc4_unorm_block;
  else if (format_name == "bc4_snorm")
    return resource_db::image_format::bc4_snorm_block;
  else if (format_name == "bc5_unorm")
    return resource_db::image_format::bc5_unorm_block;
  else if (format_name == "bc5_snorm")
    return resource_db::image_format::bc5_snorm_block;
  else if (format_name == "bc6h_ufloat")
    return resource_db::image_format::bc6h_ufloat_block;
  else if (format_name == "bc6h_sfloat")
    return resource_db::image_format::bc6h_sfloat_block;
  else if (format_name == "bc7_unorm")
    return resource_db::image_format::bc7_unorm_block;
  else if (format_name == "bc7_srgb")
    return resource_db::image_format::bc7_srgb_block;
  else
    return resource_db::image_format::undefined;
}

/// Returns the image_format corresponding to the format of the passed TIFF
/// image.
resource_db::image_format to_format(const image_util::tiff_image& image)
{
  switch (image.samples_format)
  {
  case SAMPLEFORMAT_UINT:
    switch (image.bits_per_sample)
    {
    case 8:
      switch (image.samples_per_pixel)
      {
      case 1:
        return resource_db::image_format::r8_unorm;
      case 2:
        return resource_db::image_format::r8g8_unorm;
      case 3:
        return resource_db::image_format::r8g8b8_unorm;
      case 4:
        return resource_db::image_format::r8g8b8a8_unorm;
      }
      break;

    case 16:
      switch (image.samples_per_pixel)
      {
      case 1:
        return resource_db::image_format::r16_unorm;
      case 2:
        return resource_db::image_format::r16g16_unorm;
      case 3:
        return resource_db::image_format::r16g16b16_unorm;
      case 4:
        return resource_db::image_format::r16g16b16a16_unorm;
      }
      break;
    }
    break;

  case SAMPLEFORMAT_INT:
    switch (image.bits_per_sample)
    {
    case 8:
      switch (image.samples_per_pixel)
      {
      case 1:
        return resource_db::image_format::r8_snorm;
      case 3:
        return resource_db::image_format::r8g8b8_snorm;
      case 4:
        return resource_db::image_format::r8g8b8a8_snorm;
      }
      break;

    case 16:
      switch (image.samples_per_pixel)
      {
      case 1:
        return resource_db::image_format::r16_snorm;
      case 2:
        return resource_db::image_format::r16g16_snorm;
      case 3:
        return resource_db::image_format::r16g16b16_snorm;
      case 4:
        return resource_db::image_format::r16g16b16a16_snorm;
      }
      break;
    }
    break;

  case SAMPLEFORMAT_IEEEFP:
    switch (image.bits_per_sample)
    {
    case 16:
      switch (image.samples_per_pixel)
      {
      case 1:
        return resource_db::image_format::r16_sfloat;
      case 4:
        return resource_db::image_format::r16g16b16a16_sfloat;
      }
      break;

    case 32:
      switch (image.samples_per_pixel)
      {
      case 1:
        return resource_db::image_format::r32_sfloat;
      case 4:
        return resource_db::image_format::r32g32b32a32_sfloat;
      }
      break;
    }
    break;
  }
  assert(false);
  return resource_db::image_format::undefined;
}

action_image_import_tiff::action_image_import_tiff()
: action_base(action_name, action_version)
{
}

bool action_image_import_tiff::process(resource_compiler_impl& compiler,
                                       job_description& job) const
{
  using namespace std::string_literals;

  static image_util::tiff_io io;

  /// ToDo: Add support for importing multiple 2D images to cube or 3D images.
  if (job.inputs.size() != 1)
  {
    log::error() << "The " << action_image_import_tiff::action_name
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

  auto generate_mip_maps =
    parser::json::has(job.rule->options, "generate-mip-maps")
      ? parser::json::get<bool>(job.rule->options.at("generate-mip-maps"))
      : true;
  auto ignore_icc_profile =
    parser::json::has(job.rule->options, "ignore-icc-profile")
      ? parser::json::get<bool>(job.rule->options.at("ignore-icc-profile"))
      : false;

  std::vector<image_util::tiff_image> source_images;
  if (!io.load(input.file->path, source_images, ignore_icc_profile) ||
      source_images.empty())
  {
    log::error() << "Failed loading image.";
    return false;
  }

  /// ToDo: Implement multi-image TIFF support. We need to decide what to do
  /// with each image found. It could be
  /// - a pyramidal TIFF file with a certain number of pre-computed mip maps
  ///   (usually not down to 1x1)
  /// - a cube map with 6 images, where we need to know or assume which image
  ///   should ge to which side of the cube.
  /// - a volume texture with image slices.
  /// - a set of images belonging to a single material (albedo map, normal map,
  ///   metalness map, ...)
  /// We can't automatically decide which case to select based on the
  /// information provided by standard TIFF files. Eventually introduce custom
  /// TIFF tags for this, but that would require additional tools to work with
  /// such files.
  /// A better way would be to use rule options.
  if (source_images.size() != 1)
  {
    log::error() << "We don't support multi-image TIFF files, yet.";
    return false;
  }

  auto target_format = to_format(
    parser::json::has(job.rule->options, "target-format")
      ? parser::json::get<std::string>(job.rule->options.at("target-format"))
      : "auto"s);
  if (target_format == resource_db::image_format::undefined)
    target_format = to_format(source_images.front());

  auto target_image = std::make_shared<resource_db::image>();
  for (auto& source_image : source_images)
  {
    //    // Check the size of the source image.
    //    if (!target_image->mipmaps.empty())
    //    {
    //      const auto& last_mipmap = target_image->mipmaps.back();
    //      if (source_image.width != std::max(last_mipmap.width / 2u, 1u) ||
    //          source_image.height != std::max(last_mipmap.height / 2u, 1u))
    //      {
    //        log::error() << "Wrong size of mip map image.";
    //        return false;
    //      }
    //    }

    image_util::tiff_image next_image;
    auto* current_image = &source_image;
    do
    {
      image_util::destination_image_descriptor destination;
      destination.width = current_image->width;
      destination.height = current_image->height;
      destination.row_stride = 0;
      destination.format = target_format;
      /// ToDo: required_buffer_size and new buffer.
      destination.buffer_size = current_image->pixel_data.size();
      destination.buffer = current_image->pixel_data.data();

      image_util::source_image_descriptor source;
      source.width = current_image->width;
      source.height = current_image->height;
      source.row_stride = 0;
      source.format = to_format(*current_image);
      source.buffer_size = current_image->pixel_data.size();
      source.buffer = current_image->pixel_data.data();

      image_util::convert_region region;
      region.destination_x = 0;
      region.destination_y = 0;
      region.source_x = 0;
      region.source_y = 0;
      region.width = source.width;
      region.height = source.height;

      if (auto error =
            image_util::convert_image(destination, source, region, {});
          error)
      {
        log::error() << "Image compression failed: " << error;
        return false;
      }
      if (!generate_mip_maps)
        break;
      break;  /// ToDo: remove when scale_down is back.
      // if (!scale_down(*last_image, next_image))
      // {
      //   log::error() << "Image resize failed.";
      //   return false;
      // }
      current_image = &next_image;
    } while (current_image->width > 1u || current_image->height > 1u);
  }

  std::size_t lod_level = 0;
  resource_db::mipmap_info* previous_mipmap = nullptr;
  // We need to update all buffer resource_ptr ids in a separate loop because of
  // the break in the loop below.
  for (auto& mipmap : target_image->mipmaps)
    mipmap.buffer.update_id();
  for (auto& mipmap : target_image->mipmaps)
  {
    if (previous_mipmap != nullptr &&
        mipmap.buffer.get_shared() == previous_mipmap->buffer.get_shared())
    {
      break;
    }
    compiler.save(
      *mipmap.buffer,
      job.output_file_path(
        "buffer", {std::make_pair("lod-level", std::to_string(lod_level))}),
      job);

    ++lod_level;
    previous_mipmap = &mipmap;
  }

  input.file->alias =
    compiler.save(*target_image, job.output_file_path("header", {}), job);
  return true;
}
}
