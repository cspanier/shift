#ifndef SHIFT_RC_IMAGE_TIFF_IO_HPP
#define SHIFT_RC_IMAGE_TIFF_IO_HPP

#include <cstdint>
#include <vector>
#include <filesystem>
#include <shift/core/singleton.hpp>

namespace shift::rc
{
using cms_profile = void;

/// Custom deleter for CMS profiles.
struct cms_profile_deleter
{
  void operator()(cms_profile* profile);
};

///
struct tiff_image
{
  std::uint16_t samples_per_pixel = 0;
  std::uint16_t bits_per_sample = 0;
  std::uint16_t samples_format = 0;
  std::uint16_t compression = 0;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t rows_per_strip = 0;

  std::vector<std::byte> pixel_data;
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

  /// Converts an image to linear color space.
  bool convert_to_linear(const tiff_image& source, tiff_image& target);
};
}

#endif
