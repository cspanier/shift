#ifndef SHIFT_RC_IMAGE_UTIL_TIFF_IO_HPP
#define SHIFT_RC_IMAGE_UTIL_TIFF_IO_HPP

#include <tiff.h>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <shift/core/singleton.hpp>

namespace shift::rc::image_util
{
using cms_profile = void;

/// Custom deleter for CMS profiles.
struct cms_profile_deleter
{
  void operator()(cms_profile* profile);
};

enum class tiff_icc_profile_category
{
  linear,
  srgb,
  custom
};

///
struct tiff_image
{
  tiff_image() = default;
  tiff_image(const tiff_image& other);
  tiff_image(tiff_image&&) = default;
  tiff_image& operator=(const tiff_image& other);
  tiff_image& operator=(tiff_image&&) = default;

  std::uint16_t samples_per_pixel = 0;
  std::uint16_t extra_samples = 0;
  std::uint16_t bits_per_sample = 0;
  std::uint16_t samples_format = 0;
  std::uint16_t compression = 0;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t rows_per_strip = 0;

  std::uint16_t photometric = PHOTOMETRIC_RGB;
  std::uint16_t planar_config = PLANARCONFIG_CONTIG;

  std::vector<std::byte> pixel_data;
  tiff_icc_profile_category icc_profile_category;
  std::unique_ptr<cms_profile, cms_profile_deleter> icc_profile;
};

/// A TIFF file reader and writer.
class tiff_io : public core::singleton<tiff_io, core::create::on_stack>
{
public:
  /// Constructor.
  tiff_io();

  /// Loads all contained images from a TIFF file.
  bool load(const std::filesystem::path& filename,
            std::vector<tiff_image>& images, bool ignore_icc_profile);

  /// Saves a list of images to a TIFF file.
  bool save(const std::filesystem::path& filename,
            const std::vector<tiff_image>& images, bool ignore_icc_profile);

  /// Override an image icc profile with one of the built-in profiles.
  /// @remarks
  ///   This method is useful if you do have an image with a wrong ICC profile
  ///   (e.g. a normal map with an sRGB profile assigned, which doesn't make any
  ///   sense, in which case you'd override the profile with a linear one).
  /// @param profile_category
  ///   May be one of the enum values except tiff_icc_profile_category::custom.
  bool override_icc_profile(tiff_image& image,
                            tiff_icc_profile_category profile_category);

  /// Converts an image to linear color space.
  bool convert_to_linear(const tiff_image& source, tiff_image& target);
};
}

#endif
