#include "shift/rc/action_image_import.hpp"
#include "shift/rc/resource_compiler_impl.hpp"
#include <shift/resource/image.hpp>
#include <shift/log/log.hpp>
#include <shift/math/utility.hpp>
#include <shift/math/vector.hpp>
#include <shift/core/mpl.hpp>
#include <shift/core/string_util.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/filesystem.hpp>
#define png_infopp_NULL nullptr
#define int_p_NULL nullptr
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/extension/io/jpeg.hpp>
#include <boost/gil/extension/io/tiff.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>
#include <boost/gil/extension/numeric/resample.hpp>
#include <boost/gil/gil_all.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <gsl/gsl>

namespace shift::rc
{
namespace fs = boost::filesystem;
namespace mpl = boost::mpl;
namespace gil = boost::gil;

///
enum class img_file_type
{
  png,
  jpeg,
  tiff
};

template <typename View>
void normalize(const View& image_view)
{
  gil::gil_function_requires<gil::MutableImageViewConcept<View>>();

  constexpr auto num_channels = gil::num_channels<View>::type::value;
  using channel_type = typename gil::channel_type<View>::type;

  for (std::uint32_t y = 0; y < static_cast<std::uint32_t>(image_view.height());
       ++y)
  {
    auto row_iter = image_view.row_begin(y);
    for (std::uint32_t x = 0;
         x < static_cast<std::uint32_t>(image_view.width()); ++x)
    {
      math::vector<num_channels, float> v;
      for (std::size_t c = 0; c < num_channels; ++c)
      {
        v(c) =
          row_iter[x][c] / (std::numeric_limits<channel_type>::max() + 0.5f);
      }
      v = math::normalize(v);
      for (std::size_t c = 0; c < num_channels; ++c)
      {
        row_iter[x][c] = static_cast<channel_type>(
          v(c) * (std::numeric_limits<channel_type>::max() + 0.5f));
      }
    }
  }
}

///
class image_converter
{
public:
  ///
  image_converter(resource::image& image,
                  const boost::filesystem::path& source_name,
                  img_file_type source_type, bool normalized)
  : image(image),
    source_name(source_name),
    source_type(source_type),
    normalized(normalized)
  {
  }

  ///
  template <typename Image, bool SRGB>
  bool load_and_convert()
  {
    Image source_image;
    switch (source_type)
    {
    case img_file_type::png:
      gil::read_and_convert_image(source_name.generic_string(), source_image,
                                  gil::png_tag{});
      break;
    case img_file_type::jpeg:
      gil::read_and_convert_image(source_name.generic_string(), source_image,
                                  gil::jpeg_tag{});
      break;
    case img_file_type::tiff:
      gil::read_and_convert_image(source_name.generic_string(), source_image,
                                  gil::tiff_tag{});
      break;
    }
    return copy_pixels<SRGB>(gil::const_view(source_image));
  }

  ///
  template <typename Image>
  bool load()
  {
    Image source_image;
    switch (source_type)
    {
    case img_file_type::png:
      gil::read_image(source_name.generic_string(), source_image,
                      gil::png_tag{});
      break;
    case img_file_type::jpeg:
      gil::read_image(source_name.generic_string(), source_image,
                      gil::jpeg_tag{});
      break;
    case img_file_type::tiff:
      gil::read_image(source_name.generic_string(), source_image,
                      gil::tiff_tag{});
      break;
    }
    return gil::apply_operation(gil::const_view(source_image), *this);
  }

public:
  using result_type = bool;

  ///
  template <typename SourceView>
  bool operator()(const SourceView& source_view) const
  {
    /// ToDo: Find out whether the pixels are stored in sRGB color space!
    return copy_pixels<false>(source_view);
  }

private:
  ///
  template <bool SRGB, typename SourceView>
  bool copy_pixels(const SourceView& source_view) const
  {
    using namespace shift::core::literals;
    using pixel_t = typename SourceView::value_type;

    image.format = resource::format_from_gil_pixel<pixel_t, SRGB>::value;
    image.array_element_count = 1;
    image.face_count = 1;

    std::shared_ptr<resource::buffer> previous_buffer;
    std::uint32_t buffer_offset = 0;
    for (std::uint32_t
           width = static_cast<std::uint32_t>(source_view.width()),
           height = static_cast<std::uint32_t>(source_view.height()),
           depth = 1u;
         width > 0 || height > 0 || depth > 0;
         width /= 2u, height /= 2u, depth /= 2u)
    {
      resource::mipmap_info mipmap;
      mipmap.width = std::max(width, 1u);
      mipmap.height = std::max(height, 1u);
      mipmap.depth = std::max(depth, 1u);
      auto mipmap_size =
        mipmap.width * mipmap.height * mipmap.depth * sizeof(pixel_t);

      // Each layer of at least 64KiB gets its own buffer. This way we can
      // later start streaming low quality mipmaps first and then
      // incrementally increase texture quality. Smaller mipmap layers get
      // grouped together into one buffer because the overhead of loading
      // these small textures separately is larger than simply loading all
      // of them in one go.
      /// ToDo: Make this constant configurable.
      if (mipmap_size >= 64_KiB || !previous_buffer)
      {
        // Create a new buffer for this mipmap level.
        buffer_offset = 0;
        mipmap.buffer = std::make_shared<resource::buffer>();
        previous_buffer = mipmap.buffer.get_shared();
      }
      else
      {
        buffer_offset =
          static_cast<std::uint32_t>(previous_buffer->storage.size());
        mipmap.buffer = previous_buffer;
      }
      mipmap.buffer->storage.resize(buffer_offset + mipmap_size);
      mipmap.offset = buffer_offset;
      image.mipmaps.push_back(mipmap);
    }

    resource::mipmap_info* previous_mipmap = nullptr;
    for (auto& mipmap : image.mipmaps)
    {
      if (!previous_mipmap)
      {
        gil::copy_pixels(source_view,
                         gil::interleaved_view(
                           static_cast<std::uint32_t>(source_view.width()),
                           static_cast<std::uint32_t>(source_view.height()),
                           reinterpret_cast<pixel_t*>(
                             mipmap.buffer->storage.data() + mipmap.offset),
                           static_cast<std::ptrdiff_t>(
                             static_cast<std::uint32_t>(source_view.width()) *
                             sizeof(pixel_t))));
      }
      else
      {
        gil::resize_view(
          gil::interleaved_view(
            previous_mipmap->width, previous_mipmap->height,
            reinterpret_cast<pixel_t*>(
              &previous_mipmap->buffer->storage[previous_mipmap->offset]),
            previous_mipmap->width * sizeof(pixel_t)),
          gil::interleaved_view(
            mipmap.width, mipmap.height,
            reinterpret_cast<pixel_t*>(&mipmap.buffer->storage[mipmap.offset]),
            mipmap.width * sizeof(pixel_t)),
          gil::bilinear_sampler());
      }

      if (normalized)
      {
        normalize(gil::interleaved_view(
          mipmap.width, mipmap.height,
          reinterpret_cast<pixel_t*>(mipmap.buffer->storage.data() +
                                     mipmap.offset),
          mipmap.width * sizeof(pixel_t)));
      }
      previous_mipmap = &mipmap;
    }

    return true;
  }

  resource::image& image;
  const boost::filesystem::path& source_name;
  img_file_type source_type;
  bool normalized;
};

action_image_import::action_image_import()
: action_base(action_name, action_version)
{
}

bool action_image_import::process(resource_compiler_impl& compiler,
                                  job_description& job) const
{
  using namespace std::string_literals;

  /// ToDo: Add support for importing multiple 2D images to cube or 3D images.
  if (job.inputs.size() != 1)
  {
    log::error() << "The " << action_image_import::action_name
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

  auto extension =
    core::to_lower(input.file->path.extension().generic_string());
  img_file_type source_type;
  if (extension == ".png")
    source_type = img_file_type::png;
  else if (extension == ".jpg" || extension == ".jpeg")
    source_type = img_file_type::jpeg;
  else if (extension == ".tif" || extension == ".tiff")
    source_type = img_file_type::tiff;
  else
  {
    log::error() << "Cannot auto-detect input file type: " << extension;
    return false;
  }

  auto image = std::make_shared<resource::image>();
  bool result = false;
  image_converter converter{
    *image, input.file->path, source_type,
    parser::json::has(job.rule->options, "normalize")
      ? parser::json::get<bool>(job.rule->options.at("normalize"))
      : false};
  try
  {
    if (target_format == "auto")
    {
      /// ToDo: Even though Boost GIL is quite flexible, it has very limited
      /// support for different format types (e.g. there is no grey8_alpha24
      /// format). Look for an alternative library.
      /// ToDo: GIL cannot differentiate between ARGB, RGBA, ABGR and BGRA.
      /// Instead it loads the first format that matches number and size of
      /// channels.
      /// ToDo: GIL does not have any concept of color space. Images loaded
      /// from disk lose their color space information and don't get
      /// converted to linear space either. Look for a different library!
      result = converter.load<gil::any_image<mpl::vector<
        gil::gray8_image_t, gil::gray16_image_t, gil::gray32f_image_t,
        /*gil::gray_alpha8_image_t, gil::gray_alpha16_image_t,
        gil::gray_alpha32_image_t,*/
        gil::rgb8_image_t, gil::rgb16_image_t, gil::rgb32f_image_t,
        gil::rgba8_image_t, gil::rgba16_image_t, gil::rgba32f_image_t>>>();
    }
    else if (target_format == "r8_unorm")
      result = converter.load_and_convert<gil::gray8_image_t, false>();
    else if (target_format == "r8_snorm")
      result = converter.load_and_convert<gil::gray8s_image_t, false>();
    else if (target_format == "r8_srgb")
      result = converter.load_and_convert<gil::gray8_image_t, true>();
    else if (target_format == "r16_unorm")
      result = converter.load_and_convert<gil::gray16_image_t, false>();
    else if (target_format == "r16_snorm")
      result = converter.load_and_convert<gil::gray16s_image_t, false>();
    else if (target_format == "r32_sfloat")
      result = converter.load_and_convert<gil::gray32f_image_t, false>();
    //    else if (target_format == "rg8_unorm")
    //      result = converter.load_and_convert<gil::gray_alpha8_image_t,
    //      false>();
    //    else if (target_format == "rg8_snorm")
    //      result = converter.load_and_convert<gil::gray_alpha8s_image_t,
    //      false>();
    //    else if (target_format == "rg8_srgb")
    //      result = converter.load_and_convert<gil::gray_alpha8_image_t,
    //      true>();
    //    else if (target_format == "rg16_unorm")
    //      result = converter.load_and_convert<gil::gray_alpha16_image_t,
    //      false>();
    //    else if (target_format == "rg16_snorm")
    //      result = converter.load_and_convert<gil::gray_alpha16s_image_t,
    //      false>();
    //    else if (target_format == "rg32_sfloat")
    //      result = converter.load_and_convert<gil::gray_alpha32f_image_t,
    //      false>();
    else if (target_format == "rgb8_unorm")
      result = converter.load_and_convert<gil::rgb8_image_t, false>();
    else if (target_format == "rgb8_snorm")
      result = converter.load_and_convert<gil::rgb8s_image_t, false>();
    else if (target_format == "rgb8_srgb")
      result = converter.load_and_convert<gil::rgb8_image_t, true>();
    else if (target_format == "rgb16_unorm")
      result = converter.load_and_convert<gil::rgb16_image_t, false>();
    else if (target_format == "rgb16_snorm")
      result = converter.load_and_convert<gil::rgb16s_image_t, false>();
    else if (target_format == "rgb32_sfloat")
      result = converter.load_and_convert<gil::rgb32f_image_t, false>();
    else if (target_format == "bgr8_unorm")
      result = converter.load_and_convert<gil::bgr8_image_t, false>();
    else if (target_format == "bgr8_snorm")
      result = converter.load_and_convert<gil::bgr8s_image_t, false>();
    else if (target_format == "bgr8_srgb")
      result = converter.load_and_convert<gil::bgr8_image_t, true>();
    else if (target_format == "rgba8_unorm")
      result = converter.load_and_convert<gil::rgba8_image_t, false>();
    else if (target_format == "rgba8_snorm")
      result = converter.load_and_convert<gil::rgba8s_image_t, false>();
    else if (target_format == "rgba8_srgb")
      result = converter.load_and_convert<gil::rgba8_image_t, true>();
    else if (target_format == "rgba16_unorm")
      result = converter.load_and_convert<gil::rgba16_image_t, false>();
    else if (target_format == "rgba16_snorm")
      result = converter.load_and_convert<gil::rgba16s_image_t, false>();
    else if (target_format == "rgba32_sfloat")
      result = converter.load_and_convert<gil::rgba32f_image_t, false>();
    else if (target_format == "bgra8_unorm")
      result = converter.load_and_convert<gil::bgra8_image_t, false>();
    else if (target_format == "bgra8_snorm")
      result = converter.load_and_convert<gil::bgra8s_image_t, false>();
    else if (target_format == "bgra8_srgb")
      result = converter.load_and_convert<gil::bgra8_image_t, true>();
    else
    {
      log::error() << "Illegal value '" << target_format
                   << "' for property 'target-format'";
      return false;
    }
  }
  catch (...)
  {
    log::warning() << "Failed to load image " << input.file->path;
    return false;
  }

  if (!result)
  {
    log::error() << "Failed to convert image " << input.file->path;
    return false;
  }

  std::size_t lod_level = 0;
  resource::mipmap_info* previous_mipmap = nullptr;
  // We need to update all buffer resource_ptr ids in a separate loop because of
  // the break in the loop below.
  for (auto& mipmap : image->mipmaps)
    mipmap.buffer.update_id();
  for (auto& mipmap : image->mipmaps)
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
    compiler.save(*image, job.output_file_path("header", {}), job);
  return true;
}
}
