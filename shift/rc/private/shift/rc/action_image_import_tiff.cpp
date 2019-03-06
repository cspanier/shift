#include "shift/rc/action_image_import_tiff.hpp"
#include "shift/rc/resource_compiler_impl.hpp"
#include "shift/rc/image/tiff_io.hpp"
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
#include <compressonator/Compressonator.h>
#include <tiffio.h>

namespace shift::rc
{
namespace fs = std::filesystem;
namespace gil = boost::gil;

resource_db::image_format to_format(const std::string& format_name,
                                    resource_db::image_format default_format)
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
  else if (format_name == "r32_sfloat")
    return resource_db::image_format::r32_sfloat;
  else if (format_name == "rgb8_unorm")
    return resource_db::image_format::r8g8b8_unorm;
  else if (format_name == "rgb8_snorm")
    return resource_db::image_format::r8g8b8_snorm;
  else if (format_name == "rgb8_srgb")
    return resource_db::image_format::r8g8b8_srgb;
  else if (format_name == "rgb16_unorm")
    return resource_db::image_format::r16g16b16_unorm;
  else if (format_name == "rgb16_snorm")
    return resource_db::image_format::r16g16b16_snorm;
  else if (format_name == "rgb32_sfloat")
    return resource_db::image_format::r32g32b32_sfloat;
  else if (format_name == "bgr8_unorm")
    return resource_db::image_format::b8g8r8_unorm;
  else if (format_name == "bgr8_snorm")
    return resource_db::image_format::b8g8r8_snorm;
  else if (format_name == "bgr8_srgb")
    return resource_db::image_format::b8g8r8_srgb;
  else if (format_name == "rgba8_unorm")
    return resource_db::image_format::r8g8b8a8_unorm;
  else if (format_name == "rgba8_snorm")
    return resource_db::image_format::r8g8b8a8_snorm;
  else if (format_name == "rgba8_srgb")
    return resource_db::image_format::r8g8b8a8_srgb;
  else if (format_name == "rgba16_unorm")
    return resource_db::image_format::r16g16b16a16_unorm;
  else if (format_name == "rgba16_snorm")
    return resource_db::image_format::r16g16b16a16_snorm;
  else if (format_name == "rgba32_sfloat")
    return resource_db::image_format::r32g32b32a32_sfloat;
  else if (format_name == "bgra8_unorm")
    return resource_db::image_format::b8g8r8a8_unorm;
  else if (format_name == "bgra8_snorm")
    return resource_db::image_format::b8g8r8a8_snorm;
  else if (format_name == "bgra8_srgb")
    return resource_db::image_format::b8g8r8a8_srgb;
  else
    return default_format;
}

cmp_format to_cmp_format(resource_db::image_format format)
{
  switch (format)
  {
  case resource_db::image_format::undefined:
    return cmp_format::Unknown;

  case resource_db::image_format::r8_unorm:
    return cmp_format::R_8;
  case resource_db::image_format::r8_snorm:
    return cmp_format::R_8;
  case resource_db::image_format::r8_srgb:
    return cmp_format::R_8;
  case resource_db::image_format::r16_unorm:
    return cmp_format::R_16;
  case resource_db::image_format::r16_snorm:
    return cmp_format::R_16;
  case resource_db::image_format::r16_sfloat:
    return cmp_format::R_16F;
  case resource_db::image_format::r32_sfloat:
    return cmp_format::R_32F;

  case resource_db::image_format::r8g8_unorm:
    return cmp_format::RG_8;
  case resource_db::image_format::r8g8_snorm:
    return cmp_format::RG_8;
  case resource_db::image_format::r8g8_srgb:
    return cmp_format::RG_8;
  case resource_db::image_format::r16g16_unorm:
    return cmp_format::RG_16;
  case resource_db::image_format::r16g16_snorm:
    return cmp_format::RG_16;
  case resource_db::image_format::r16g16_sfloat:
    return cmp_format::RG_16F;
  case resource_db::image_format::r32g32_sfloat:
    return cmp_format::RG_32F;

  case resource_db::image_format::r8g8b8_unorm:
    return cmp_format::RGB_888;
  case resource_db::image_format::r8g8b8_snorm:
    return cmp_format::RGB_888;
  case resource_db::image_format::r8g8b8_srgb:
    return cmp_format::RGB_888;
  case resource_db::image_format::r16g16b16_unorm:
    /// ToDo: Add support for this format.
    return cmp_format::Unknown;
  case resource_db::image_format::r16g16b16_snorm:
    /// ToDo: Add support for this format.
    return cmp_format::Unknown;
  case resource_db::image_format::r16g16b16_sfloat:
    /// ToDo: Add support for this format.
    return cmp_format::Unknown;
  case resource_db::image_format::r32g32b32_sfloat:
    return cmp_format::RGB_32F;

  case resource_db::image_format::b8g8r8_unorm:
    return cmp_format::BGR_888;
  case resource_db::image_format::b8g8r8_snorm:
    return cmp_format::BGR_888;
  case resource_db::image_format::b8g8r8_srgb:
    return cmp_format::BGR_888;

  case resource_db::image_format::r8g8b8a8_unorm:
    return cmp_format::RGBA_8888;
  case resource_db::image_format::r8g8b8a8_snorm:
    return cmp_format::RGBA_8888;
  case resource_db::image_format::r8g8b8a8_srgb:
    return cmp_format::RGBA_8888;
  case resource_db::image_format::r16g16b16a16_unorm:
    return cmp_format::RGBA_16;
  case resource_db::image_format::r16g16b16a16_snorm:
    return cmp_format::RGBA_16;
  case resource_db::image_format::r16g16b16a16_sfloat:
    return cmp_format::RGBA_16F;
  case resource_db::image_format::r32g32b32a32_sfloat:
    return cmp_format::RGBA_32F;

  case resource_db::image_format::b8g8r8a8_unorm:
    return cmp_format::BGRA_8888;
  case resource_db::image_format::b8g8r8a8_snorm:
    return cmp_format::BGRA_8888;
  case resource_db::image_format::b8g8r8a8_srgb:
    return cmp_format::BGRA_8888;

  case resource_db::image_format::a8b8g8r8_unorm:
    return cmp_format::ABGR_8888;
  case resource_db::image_format::a8b8g8r8_snorm:
    return cmp_format::ABGR_8888;

  // S3TC Compressed Texture Image Formats
  case resource_db::image_format::bc1_rgb_unorm_block:
    return cmp_format::BC1;
  case resource_db::image_format::bc1_rgb_srgb_block:
    return cmp_format::BC1;
  case resource_db::image_format::bc1_rgba_unorm_block:
    return cmp_format::BC1;
  case resource_db::image_format::bc1_rgba_srgb_block:
    return cmp_format::BC1;
  case resource_db::image_format::bc2_unorm_block:
    return cmp_format::BC2;
  case resource_db::image_format::bc2_srgb_block:
    return cmp_format::BC2;
  case resource_db::image_format::bc3_unorm_block:
    return cmp_format::BC3;
  case resource_db::image_format::bc3_srgb_block:
    return cmp_format::BC3;

  // RGTC Compressed Texture Image Formats
  case resource_db::image_format::bc4_unorm_block:
    return cmp_format::BC4;
  case resource_db::image_format::bc4_snorm_block:
    return cmp_format::BC4;
  case resource_db::image_format::bc5_unorm_block:
    return cmp_format::BC5;
  case resource_db::image_format::bc5_snorm_block:
    return cmp_format::BC5;

  // BPTC Compressed Texture Image Formats
  case resource_db::image_format::bc6h_ufloat_block:
    return cmp_format::BC6H;
  case resource_db::image_format::bc6h_sfloat_block:
    return cmp_format::BC6H;
  case resource_db::image_format::bc7_unorm_block:
    return cmp_format::BC7;
  case resource_db::image_format::bc7_srgb_block:
    return cmp_format::BC7;
  }

  assert(false);
  return cmp_format::Unknown;
}

resource_db::image_format to_format(const tiff_image& image)
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
      case 4:
        return resource_db::image_format::r16g16b16a16_unorm;
      }
      break;
    }
    break;

  case SAMPLEFORMAT_INT:
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

cmp_format to_cmp_format(const tiff_image& image)
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
        return cmp_format::R_8;
      case 3:
        return cmp_format::RGB_888;
      case 4:
        return cmp_format::RGBA_8888;
      }
      break;

    case 16:
      switch (image.samples_per_pixel)
      {
      case 1:
        return cmp_format::R_16;
      case 4:
        return cmp_format::RGBA_16;
      }
      break;
    }
    break;

  case SAMPLEFORMAT_INT:
    break;

  case SAMPLEFORMAT_IEEEFP:
    switch (image.bits_per_sample)
    {
    case 16:
      switch (image.samples_per_pixel)
      {
      case 1:
        return cmp_format::R_16F;
      case 4:
        return cmp_format::RGBA_16F;
      }
      break;

    case 32:
      switch (image.samples_per_pixel)
      {
      case 1:
        return cmp_format::R_32F;
      case 4:
        return cmp_format::RGBA_32F;
      }
      break;
    }
    break;
  }
  assert(false);
  return cmp_format::Unknown;
}

/// Compresses the source image using AMD Compressonator and store the result as
/// a new mip map in target.
bool compress(const tiff_image& source, resource_db::image& target,
              resource_db::image_format target_format)
{
  using namespace shift::core::literals;

  CMP_Texture source_texture;
  source_texture.width = source.width;
  source_texture.height = source.height;
  source_texture.pitch = 0;
  source_texture.format = to_cmp_format(source);
  /// ToDo: Why 32bit only?
  source_texture.data_size =
    static_cast<std::uint32_t>(source.pixel_data.size());
  /// ToDo: Make use of std::byte.
  /// ToDo: Get rid of const_cast.
  source_texture.data = reinterpret_cast<std::uint8_t*>(
    const_cast<std::byte*>(source.pixel_data.data()));

  CMP_Texture target_texture;
  target_texture.width = source.width;
  target_texture.height = source.height;
  target_texture.pitch = 0;
  target_texture.format = to_cmp_format(target_format);

  std::vector<std::byte> target_texture_data;
  target_texture_data.resize(CMP_CalculateBufferSize(&target_texture));
  /// ToDo: Why 32bit only?
  target_texture.data_size =
    static_cast<std::uint32_t>(target_texture_data.size());
  /// ToDo: Make use of std::byte
  target_texture.data =
    reinterpret_cast<std::uint8_t*>(target_texture_data.data());

  CMP_CompressOptions options{};
  options.fquality = 0.05f;
  options.dwnumThreads = 16;

  CMP_ERROR status = CMP_ConvertTexture(&source_texture, &target_texture,
                                        &options, nullptr, 0, 0);
  if (status != CMP_OK)
    return false;

  resource_db::mipmap_info mipmap;
  mipmap.width = target_texture.width;
  mipmap.height = target_texture.height;
  mipmap.depth = 1u;

  // Each layer of at least 64KiB gets its own buffer. This way we can
  // later start streaming low quality mipmaps first and then
  // incrementally increase texture quality. Smaller mipmap layers get
  // grouped together into one buffer because the overhead of loading
  // these small textures separately is larger than simply loading all
  // of them in one go.
  /// ToDo: Make this size constant configurable.
  std::shared_ptr<resource_db::buffer> previous_buffer;
  if (!target.mipmaps.empty())
    previous_buffer = target.mipmaps.back().buffer.get_shared();
  if (target_texture.data_size >= 64_KiB || !previous_buffer)
  {
    // Create a new buffer for this mipmap level.
    mipmap.offset = 0;
    mipmap.buffer = std::make_shared<resource_db::buffer>();
    previous_buffer = mipmap.buffer.get_shared();
  }
  else
  {
    mipmap.offset = static_cast<std::uint32_t>(previous_buffer->storage.size());
    mipmap.buffer = previous_buffer;
  }
  mipmap.buffer->storage.resize(mipmap.offset + target_texture.data_size);
  target.mipmaps.push_back(mipmap);
  target.format = target_format;

  return true;
}

///
bool scale_down_ex(const tiff_image& source, tiff_image& target)
{
  tiff_image temp_image;
  temp_image.samples_per_pixel = source.samples_per_pixel;
  temp_image.bits_per_sample = source.bits_per_sample;
  temp_image.samples_format = source.samples_format;
  temp_image.compression = source.compression;
  temp_image.width = source.width / 2u;
  temp_image.height = source.height / 2u;
  temp_image.rows_per_strip =
    std::min(source.rows_per_strip * 4u, target.height);
  temp_image.pixel_data.resize(temp_image.width * temp_image.height *
                               temp_image.samples_per_pixel *
                               temp_image.bits_per_sample / 8u);

  target = std::move(temp_image);
  return true;
}

template <typename Pixel>
bool scale_down_impl(const tiff_image& source, tiff_image& target)
{
  gil::resize_view(
    gil::interleaved_view(
      source.width, source.height,
      reinterpret_cast<const Pixel*>(source.pixel_data.data()),
      source.width * sizeof(Pixel)),
    gil::interleaved_view(target.width, target.height,
                          reinterpret_cast<Pixel*>(target.pixel_data.data()),
                          target.width * sizeof(Pixel)),
    gil::bilinear_sampler());
  return true;
}

///
bool scale_down(const tiff_image& source, tiff_image& target)
{
  tiff_image temp_image;
  temp_image.samples_per_pixel = source.samples_per_pixel;
  temp_image.bits_per_sample = source.bits_per_sample;
  temp_image.samples_format = source.samples_format;
  temp_image.compression = source.compression;
  temp_image.width = std::max(source.width / 2u, 1u);
  temp_image.height = std::max(source.height / 2u, 1u);
  temp_image.rows_per_strip =
    std::min(source.rows_per_strip * 4u, temp_image.height);
  temp_image.pixel_data.resize(temp_image.width * temp_image.height *
                               temp_image.samples_per_pixel *
                               temp_image.bits_per_sample / 8u);

  bool result = false;
  switch (source.samples_format)
  {
  case SAMPLEFORMAT_UINT:
    switch (source.bits_per_sample)
    {
    case 8:
      switch (source.samples_per_pixel)
      {
      case 1:
        result = scale_down_impl<gil::gray8_pixel_t>(source, temp_image);
        break;
      case 3:
        result = scale_down_impl<gil::rgb8_pixel_t>(source, temp_image);
        break;
      case 4:
        result = scale_down_impl<gil::rgba8_pixel_t>(source, temp_image);
        break;
      }
      break;

    case 16:
      switch (source.samples_per_pixel)
      {
      case 1:
        result = scale_down_impl<gil::gray16_pixel_t>(source, temp_image);
        break;
      case 3:
        result = scale_down_impl<gil::rgb16_pixel_t>(source, temp_image);
        break;
      case 4:
        result = scale_down_impl<gil::rgba16_pixel_t>(source, temp_image);
        break;
      }
      break;

    case 32:
      switch (source.samples_per_pixel)
      {
      case 1:
        result = scale_down_impl<gil::gray32_pixel_t>(source, temp_image);
        break;
      case 3:
        result = scale_down_impl<gil::rgb32_pixel_t>(source, temp_image);
        break;
      case 4:
        result = scale_down_impl<gil::rgba32_pixel_t>(source, temp_image);
        break;
      }
      break;
    }
    break;

  case SAMPLEFORMAT_INT:
    switch (source.bits_per_sample)
    {
    case 8:
      switch (source.samples_per_pixel)
      {
      case 1:
        result = scale_down_impl<gil::gray8s_pixel_t>(source, temp_image);
        break;
      case 3:
        result = scale_down_impl<gil::rgb8s_pixel_t>(source, temp_image);
        break;
      case 4:
        result = scale_down_impl<gil::rgba8s_pixel_t>(source, temp_image);
        break;
      }
      break;

    case 16:
      switch (source.samples_per_pixel)
      {
      case 1:
        result = scale_down_impl<gil::gray16s_pixel_t>(source, temp_image);
        break;
      case 3:
        result = scale_down_impl<gil::rgb16s_pixel_t>(source, temp_image);
        break;
      case 4:
        result = scale_down_impl<gil::rgba16s_pixel_t>(source, temp_image);
        break;
      }
      break;

    case 32:
      switch (source.samples_per_pixel)
      {
      case 1:
        result = scale_down_impl<gil::gray32s_pixel_t>(source, temp_image);
        break;
      case 3:
        result = scale_down_impl<gil::rgb32s_pixel_t>(source, temp_image);
        break;
      case 4:
        result = scale_down_impl<gil::rgba32s_pixel_t>(source, temp_image);
        break;
      }
      break;
    }
    break;

  case SAMPLEFORMAT_IEEEFP:
    switch (source.bits_per_sample)
    {
    case 16:
      /// ToDo: Find solution for missing 16bit float support in Boost GIL.
      break;

    case 32:
      switch (source.samples_per_pixel)
      {
      case 1:
        result = scale_down_impl<gil::gray32f_pixel_t>(source, temp_image);
        break;
      case 3:
        result = scale_down_impl<gil::rgb32f_pixel_t>(source, temp_image);
        break;
      case 4:
        result = scale_down_impl<gil::rgba32f_pixel_t>(source, temp_image);
        break;
      }
      break;
    }
    break;
  }
  assert(result);
  if (!result)
    return false;

  target = std::move(temp_image);
  return true;
}

action_image_import_tiff::action_image_import_tiff()
: action_base(action_name, action_version)
{
}

bool action_image_import_tiff::process(resource_compiler_impl& compiler,
                                       job_description& job) const
{
  using namespace std::string_literals;

  static tiff_io io;

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

  std::vector<tiff_image> source_images;
  if (!io.load(input.file->path, source_images, ignore_icc_profile) || source_images.empty())
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
  /// A beeter way would be to use rule options.
  if (source_images.size() != 1)
  {
    log::error() << "We don't support multi-image TIFF files, yet.";
    return false;
  }

  auto target_format = to_format(
    parser::json::has(job.rule->options, "target-format")
      ? parser::json::get<std::string>(job.rule->options.at("target-format"))
      : "auto"s,
    to_format(source_images.front()));

  auto target_image = std::make_shared<resource_db::image>();
  for (auto& source_image : source_images)
  {
    // Check the size of the source image.
    if (!target_image->mipmaps.empty())
    {
      const auto& last_mipmap = target_image->mipmaps.back();
      if (source_image.width != std::max(last_mipmap.width / 2u, 1u) ||
          source_image.height != std::max(last_mipmap.height / 2u, 1u))
      {
        log::error() << "Wrong size of mip map image.";
        return false;
      }
    }

    if (!compress(source_image, *target_image, target_format))
    {
      log::error() << "Image compression failed.";
      return false;
    }

    tiff_image next_image;
    auto* last_image = &source_image;
    while (generate_mip_maps &&
           (last_image->width > 1u || last_image->height > 1u))
    {
      if (!scale_down(*last_image, next_image))
      {
        log::error() << "Image resize failed.";
        return false;
      }
      if (!compress(*last_image, *target_image, target_format))
      {
        log::error() << "Image compression failed.";
        return false;
      }
      last_image = &next_image;
    }
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
