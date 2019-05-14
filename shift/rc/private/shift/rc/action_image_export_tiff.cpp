#include "shift/rc/action_image_export_tiff.hpp"
#include "shift/rc/resource_compiler_impl.hpp"
#include "shift/rc/image/tiff_io.hpp"
#include <shift/resource_db/image.hpp>
#include <shift/log/log.hpp>
#include <shift/math/utility.hpp>
#include <shift/math/vector.hpp>
#include <shift/core/mpl.hpp>
#include <shift/core/string_util.hpp>
#include <filesystem>
#include <gsl/gsl>
//#include <compressonator/Compressonator.h>
#include <tiffio.h>

namespace shift::rc
{
namespace fs = std::filesystem;

action_image_export_tiff::action_image_export_tiff()
: action_base(action_name, action_version)
{
}

bool action_image_export_tiff::process(resource_compiler_impl& compiler,
                                       job_description& job) const
{
  using namespace std::string_literals;

  static tiff_io io;

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

  auto target_format =
    parser::json::has(job.rule->options, "target-format")
      ? parser::json::get<std::string>(job.rule->options.at("target-format"))
      : "auto"s;

  auto image = std::make_shared<resource_db::image>();
  bool result = false;

  std::vector<tiff_image> images;
  if (!io.load(input.file->path, images, false))
  {
    log::error() << "Failed loading image.";
    return false;
  }

  //    image.samples_per_pixel = 4;
  //    auto pixel_size = image.samples_per_pixel * image.bits_per_sample /
  //    8u; image.pixel_data.resize(image.width * image.height * pixel_size);

  //    source.format = cmp_format::RGBA_8888;
  //    source.data_size =
  //    static_cast<std::uint32_t>(image.pixel_data.size()); source.data =
  //    reinterpret_cast<std::uint8_t*>(image.pixel_data.data());

  //    status = CMP_ConvertTexture(&compressed, &source, &options, nullptr,
  //    0, 0);

  //    image.compression = COMPRESSION_ZSTD;

  if (!result)
  {
    log::error() << "Failed to convert image " << input.file->path;
    return false;
  }

  //  std::size_t lod_level = 0;
  //  resource_db::mipmap_info* previous_mipmap = nullptr;
  //  // We need to update all buffer resource_ptr ids in a separate loop
  //  because of
  //  // the break in the loop below.
  //  for (auto& mipmap : image->mipmaps)
  //    mipmap.buffer.update_id();
  //  for (auto& mipmap : image->mipmaps)
  //  {
  //    if (previous_mipmap != nullptr &&
  //        mipmap.buffer.get_shared() == previous_mipmap->buffer.get_shared())
  //    {
  //      break;
  //    }
  //    compiler.save(
  //      *mipmap.buffer,
  //      job.output_file_path(
  //        "buffer", {std::make_pair("lod-level", std::to_string(lod_level))}),
  //      job);

  //    ++lod_level;
  //    previous_mipmap = &mipmap;
  //  }

  input.file->alias =
    compiler.save(*image, job.output_file_path("header", {}), job);
  return true;
}
}
