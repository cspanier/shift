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

/// Enumeration of supported TIFF image samples formats.
enum class tiff_samples_format : std::uint16_t
{
  /// @see SAMPLEFORMAT_UINT
  unsigned_int = 1,
  /// @see SAMPLEFORMAT_INT
  signed_int = 2,
  /// @see SAMPLEFORMAT_IEEEFP
  floating_point = 3,
  /// @see SAMPLEFORMAT_VOID
  untyped = 4,
  /// @see SAMPLEFORMAT_COMPLEXINT
  complex_signed_int = 5,
  /// @see SAMPLEFORMAT_COMPLEXIEEEFP
  complex_floating_point = 6,
  /// Some other illegal value to get rid of warnings in switch case statements.
  other = 0xFFFF
};

/// Enumeration of supported TIFF image compression formats.
enum class tiff_compression : std::uint16_t
{
  /// @see COMPRESSION_NONE,
  none = 1,
  /// @see COMPRESSION_CCITTRLE
  ccittrle = 2,
  /// @see COMPRESSION_CCITTFAX3
  ccittfax3 = 3,
  /// @see COMPRESSION_CCITTFAX4
  ccittfax4 = 4,
  /// @see COMPRESSION_LZW
  lzw = 5,
  /// @see COMPRESSION_JPEG
  jpeg = 7,
  /// @see COMPRESSION_ADOBE_DEFLATE
  adobe_deflate = 8,
  /// @see COMPRESSION_PACKBITS
  packbits = 32773,
  /// @see COMPRESSION_LZMA
  lzma = 34925,
  /// @see COMPRESSION_ZSTD
  zstd = 50000,
  /// @see COMPRESSION_WEBP
  webp = 50001,
  /// Some other illegal value to get rid of warnings in switch case statements.
  other = 0xFFFF
};

/// Enumeration of supported TIFF photometric formats.
enum class tiff_photometric : std::uint16_t
{
  /// @see PHOTOMETRIC_MINISWHITE
  min_is_white = 0,
  /// @see PHOTOMETRIC_MINISBLACK
  min_is_black = 1,
  /// @see PHOTOMETRIC_RGB
  rgb = 2,
  /// @see PHOTOMETRIC_PALETTE
  palette = 3,
  /// @see PHOTOMETRIC_SEPARATED
  separated = 5,
  /// @see PHOTOMETRIC_YCBCR
  ycbcr = 6,
  /// @see PHOTOMETRIC_CIELAB
  cie_lab = 8,
  /// @see PHOTOMETRIC_ICCLAB
  icc_lab = 9,
  /// @see PHOTOMETRIC_LOGLUV
  log_luv = 32845,
  /// Some other illegal value to get rid of warnings in switch case statements.
  other = 0xFFFF
};

enum class tiff_planar_config : std::uint16_t
{
  /// Single image plane.
  /// @see PLANARCONFIG_CONTIG
  contiguous = 1,
  /// Separate planes of data.
  /// @see PLANARCONFIG_SEPARATE
  separate = 2,
  /// Some other illegal value to get rid of warnings in switch case statements.
  other = 0xFFFF
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
  tiff_samples_format samples_format = tiff_samples_format::unsigned_int;
  tiff_compression compression = tiff_compression::none;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t rows_per_strip = 0;

  tiff_photometric photometric = tiff_photometric::rgb;
  tiff_planar_config planar_config = tiff_planar_config::contiguous;

  std::vector<std::byte> pixel_data;
  tiff_icc_profile_category icc_profile_category =
    tiff_icc_profile_category::srgb;
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
