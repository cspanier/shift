#include "shift/rc/image_util/tiff_io.hpp"
#include "shift/rc/image_util/built_in_icc_profiles.hpp"
#include <shift/log/log.hpp>
#include <fstream>
#include <array>
#include <tiffio.h>
#define register
#include <lcms2_plugin.h>
#undef register

namespace shift::rc::image_util
{
static_assert(core::underlying_type_cast(tiff_samples_format::unsigned_int) ==
              SAMPLEFORMAT_UINT);
static_assert(core::underlying_type_cast(tiff_samples_format::signed_int) ==
              SAMPLEFORMAT_INT);
static_assert(core::underlying_type_cast(tiff_samples_format::floating_point) ==
              SAMPLEFORMAT_IEEEFP);
static_assert(core::underlying_type_cast(tiff_samples_format::untyped) ==
              SAMPLEFORMAT_VOID);
static_assert(
  core::underlying_type_cast(tiff_samples_format::complex_signed_int) ==
  SAMPLEFORMAT_COMPLEXINT);
static_assert(
  core::underlying_type_cast(tiff_samples_format::complex_floating_point) ==
  SAMPLEFORMAT_COMPLEXIEEEFP);

static_assert(core::underlying_type_cast(tiff_compression::none) ==
              COMPRESSION_NONE);
static_assert(core::underlying_type_cast(tiff_compression::ccittrle) ==
              COMPRESSION_CCITTRLE);
static_assert(core::underlying_type_cast(tiff_compression::ccittfax3) ==
              COMPRESSION_CCITTFAX3);
static_assert(core::underlying_type_cast(tiff_compression::ccittfax4) ==
              COMPRESSION_CCITTFAX4);
static_assert(core::underlying_type_cast(tiff_compression::lzw) ==
              COMPRESSION_LZW);
static_assert(core::underlying_type_cast(tiff_compression::jpeg) ==
              COMPRESSION_JPEG);
static_assert(core::underlying_type_cast(tiff_compression::adobe_deflate) ==
              COMPRESSION_ADOBE_DEFLATE);
static_assert(core::underlying_type_cast(tiff_compression::packbits) ==
              COMPRESSION_PACKBITS);
static_assert(core::underlying_type_cast(tiff_compression::lzma) ==
              COMPRESSION_LZMA);
static_assert(core::underlying_type_cast(tiff_compression::zstd) ==
              COMPRESSION_ZSTD);
static_assert(core::underlying_type_cast(tiff_compression::webp) ==
              COMPRESSION_WEBP);

static_assert(core::underlying_type_cast(tiff_photometric::min_is_white) ==
              PHOTOMETRIC_MINISWHITE);
static_assert(core::underlying_type_cast(tiff_photometric::min_is_black) ==
              PHOTOMETRIC_MINISBLACK);
static_assert(core::underlying_type_cast(tiff_photometric::rgb) ==
              PHOTOMETRIC_RGB);
static_assert(core::underlying_type_cast(tiff_photometric::palette) ==
              PHOTOMETRIC_PALETTE);
static_assert(core::underlying_type_cast(tiff_photometric::separated) ==
              PHOTOMETRIC_SEPARATED);
static_assert(core::underlying_type_cast(tiff_photometric::ycbcr) ==
              PHOTOMETRIC_YCBCR);
static_assert(core::underlying_type_cast(tiff_photometric::cie_lab) ==
              PHOTOMETRIC_CIELAB);
static_assert(core::underlying_type_cast(tiff_photometric::icc_lab) ==
              PHOTOMETRIC_ICCLAB);
static_assert(core::underlying_type_cast(tiff_photometric::log_luv) ==
              PHOTOMETRIC_LOGLUV);

static_assert(core::underlying_type_cast(tiff_planar_config::contiguous) ==
              PLANARCONFIG_CONTIG);
static_assert(core::underlying_type_cast(tiff_planar_config::separate) ==
              PLANARCONFIG_SEPARATE);

static void tiff_warning(thandle_t, const char* function, const char* message,
                         va_list)
{
  log::warning() << "\"" << function << "\": " << message;
}

static void tiff_error(thandle_t, const char* function, const char* message,
                       va_list)
{
  log::error() << "\"" << function << "\": " << message;
}

void cms_profile_deleter::operator()(cms_profile* profile)
{
  static_assert(std::is_same_v<cms_profile*, cmsHPROFILE>);
  cmsCloseProfile(profile);
}

tiff_image::tiff_image(const tiff_image& other)
: samples_per_pixel(other.samples_per_pixel),
  extra_samples(other.extra_samples),
  bits_per_sample(other.bits_per_sample),
  samples_format(other.samples_format),
  compression(other.compression),
  width(other.width),
  height(other.height),
  rows_per_strip(other.rows_per_strip),
  photometric(other.photometric),
  planar_config(other.planar_config),
  pixel_data(other.pixel_data),
  icc_profile_category(other.icc_profile_category)
{
  if (other.icc_profile)
  {
    std::vector<std::byte> icc_buffer;
    std::uint32_t icc_buffer_size = 0;
    cmsSaveProfileToMem(other.icc_profile.get(), nullptr, &icc_buffer_size);
    icc_buffer.resize(icc_buffer_size);
    cmsSaveProfileToMem(other.icc_profile.get(), icc_buffer.data(),
                        &icc_buffer_size);

    icc_profile.reset(
      cmsOpenProfileFromMem(icc_buffer.data(), icc_buffer_size));
  }
}

tiff_image& tiff_image::operator=(const tiff_image& other)
{
  samples_per_pixel = other.samples_per_pixel;
  extra_samples = other.extra_samples;
  bits_per_sample = other.bits_per_sample;
  samples_format = other.samples_format;
  compression = other.compression;
  width = other.width;
  height = other.height;
  rows_per_strip = other.rows_per_strip;
  photometric = other.photometric;
  planar_config = other.planar_config;
  pixel_data = other.pixel_data;
  icc_profile_category = other.icc_profile_category;
  if (other.icc_profile)
  {
    std::vector<std::byte> icc_buffer;
    std::uint32_t icc_buffer_size = 0;
    cmsSaveProfileToMem(other.icc_profile.get(), nullptr, &icc_buffer_size);
    icc_buffer.resize(icc_buffer_size);
    cmsSaveProfileToMem(other.icc_profile.get(), icc_buffer.data(),
                        &icc_buffer_size);

    icc_profile.reset(
      cmsOpenProfileFromMem(icc_buffer.data(), icc_buffer_size));
  }
  return *this;
}

static std::uint32_t cms_pixel_type_from_sample_count(
  std::uint32_t color_samples)
{
  switch (color_samples)
  {
  case 1:
    return PT_GRAY;
  case 2:
    return PT_MCH2;
  case 3:
    return PT_MCH3;
  case 4:
    return PT_CMYK;
  case 5:
    return PT_MCH5;
  case 6:
    return PT_MCH6;
  case 7:
    return PT_MCH7;
  case 8:
    return PT_MCH8;
  case 9:
    return PT_MCH9;
  case 10:
    return PT_MCH10;
  case 11:
    return PT_MCH11;
  case 12:
    return PT_MCH12;
  case 13:
    return PT_MCH13;
  case 14:
    return PT_MCH14;
  case 15:
    return PT_MCH15;

  default:
    log::error() << "What a weird separation of " << color_samples
                 << " channels.";
    return PT_ANY;
  }
}

static std::uint32_t cms_get_input_pixel_type(const tiff_image& image)
{
  using namespace shift::rc::image_util;

  std::uint32_t is_planar;
  switch (image.planar_config)
  {
  case tiff_planar_config::contiguous:
    is_planar = 0;
    break;

  case tiff_planar_config::separate:
    is_planar = 1;
    break;

  default:
    BOOST_ASSERT(false);
    return 0;
  }

  // Reset planar configuration if the number of samples per pixel == 1.
  if (image.samples_per_pixel == 1)
    is_planar = 0;

  std::uint32_t extra_samples;
  std::uint32_t color_samples;
  if (false)  // if (StoreAsAlpha)
  {
    // Read alpha channels as colorant
    extra_samples = 0;
    color_samples = image.samples_per_pixel;
  }
  else
  {
    extra_samples = image.extra_samples;
    color_samples = image.samples_per_pixel - extra_samples;
  }

  std::uint32_t pixel_type = 0;
  std::uint32_t reverse = 0;
  std::uint32_t lab_tiff_special = 0;
  switch (image.photometric)
  {
  case tiff_photometric::min_is_white:
    reverse = 1;
    [[fallthrough]];

  case tiff_photometric::min_is_black:
    pixel_type = PT_GRAY;
    break;

  case tiff_photometric::rgb:
    pixel_type = PT_RGB;
    break;

  case tiff_photometric::separated:
    pixel_type = cms_pixel_type_from_sample_count(color_samples);
    break;

  case tiff_photometric::ycbcr:
    pixel_type = PT_YCbCr;
    break;

  case tiff_photometric::cie_lab:
    pixel_type = PT_Lab;
    lab_tiff_special = 1;
    break;

  case tiff_photometric::icc_lab:
    pixel_type = PT_LabV2;
    break;

  case tiff_photometric::log_luv:
    pixel_type = PT_YUV;
    break;

  default:
    BOOST_ASSERT(false);
    return 0;
  }

  std::uint32_t bytes_per_sample = image.bits_per_sample >> 3;
  std::uint32_t is_float =
    (image.samples_format == tiff_samples_format::floating_point) ? 1 : 0;

  return FLOAT_SH(is_float) | COLORSPACE_SH(pixel_type) | PLANAR_SH(is_planar) |
         EXTRA_SH(extra_samples) | CHANNELS_SH(color_samples) |
         BYTES_SH(bytes_per_sample) | FLAVOR_SH(reverse) |
         (lab_tiff_special << 23);
}

// static std::uint32_t cms_chan_count_from_pixel_type(std::uint32_t
// color_space)
//{
//  switch (color_space)
//  {

//  case PT_GRAY:
//    return 1;

//  case PT_MCH2:
//    return 2;

//  case PT_RGB:
//  case PT_CMY:
//  case PT_Lab:
//  case PT_YUV:
//  case PT_YCbCr:
//  case PT_MCH3:
//    return 3;

//  case PT_CMYK:
//  case PT_MCH4:
//    return 4;

//  case PT_MCH5:
//    return 5;
//  case PT_MCH6:
//    return 6;
//  case PT_MCH7:
//    return 7;
//  case PT_MCH8:
//    return 8;
//  case PT_MCH9:
//    return 9;
//  case PT_MCH10:
//    return 10;
//  case PT_MCH11:
//    return 11;
//  case PT_MCH12:
//    return 12;
//  case PT_MCH13:
//    return 12;
//  case PT_MCH14:
//    return 14;
//  case PT_MCH15:
//    return 15;

//  default:
//    BOOST_ASSERT(false);
//    // FatalError("Unsupported color space of %d channels", color_space);
//    return 0;
//  }
//}

// Rearrange pixel type to build output descriptor
tiff_io::tiff_io()
{
  TIFFSetWarningHandlerExt(tiff_warning);
  TIFFSetErrorHandlerExt(tiff_error);

#if 0
  log::info() << "List of supported TIFF compression codecs:";
  TIFFCodec* codecs = TIFFGetConfiguredCODECs();
  for (TIFFCodec* codec = codecs; codec->name != nullptr; ++codec)
  {
    switch (codec->scheme)
    {
    case COMPRESSION_PACKBITS:
      log::info() << "  PACKBITS";
      break;
    case COMPRESSION_JPEG:
      log::info() << "  JPEG";
      break;
    case COMPRESSION_LZW:
      log::info() << "  LZW";
      break;
    case COMPRESSION_ADOBE_DEFLATE:
      log::info() << "  ADOBE_DEFLATE";
      break;
    case COMPRESSION_CCITTRLE:
      log::info() << "  CCITTRLE";
      break;
    case COMPRESSION_CCITTFAX3:
      log::info() << "  CCITTFAX3";
      break;
    case COMPRESSION_CCITTFAX4:
      log::info() << "  CCITTFAX4";
      break;
    case COMPRESSION_LZMA:
      log::info() << "  LZMA";
      break;
#if defined(COMPRESSION_ZSTD)
    case COMPRESSION_ZSTD:
      log::info() << "  ZSTD";
      break;
#endif
#if defined(COMPRESSION_WEBP)
    case COMPRESSION_WEBP:
      log::info() << "  WEBP";
      break;
#endif
    default:
      break;
    }
  }
  _TIFFfree(codecs);
#endif
}

bool tiff_io::load(const std::filesystem::path& filename,
                   std::vector<tiff_image>& images, bool ignore_icc_profile)
{
  struct read_context_t
  {
    std::ifstream stream;
    toff_t start_offset = 0;
  };

  static auto tiff_read = [](thandle_t user_data, tdata_t buffer,
                             tsize_t size) -> tsize_t {
    auto* data = static_cast<read_context_t*>(user_data);

    data->stream.read(static_cast<char*>(buffer),
                      static_cast<std::streamsize>(size));
    return data->stream.gcount();
  };

  static auto tiff_write = [](thandle_t /*user_data*/, tdata_t /*buffer*/,
                              tsize_t /*size*/) -> tsize_t {
    // Disallow writing on read-only images.
    return 0;
  };

  static auto tiff_close = [](thandle_t /*user_data*/) -> int {
    // NOP.
    return 0;
  };

  static auto tiff_seek = [](thandle_t user_data, toff_t offset,
                             int whence) -> toff_t {
    if (offset == std::numeric_limits<toff_t>::max())
      return std::numeric_limits<toff_t>::max();

    auto* data = static_cast<read_context_t*>(user_data);

    switch (whence)
    {
    case SEEK_SET:
      data->stream.seekg(
        static_cast<std::streamoff>(data->start_offset + offset),
        std::ios::beg);
      break;

    case SEEK_CUR:
      data->stream.seekg(static_cast<std::streamoff>(offset), std::ios::cur);
      break;

    case SEEK_END:
      data->stream.seekg(static_cast<std::streamoff>(offset), std::ios::end);
      break;
    }

    return static_cast<toff_t>(data->stream.tellg()) - data->start_offset;
  };

  static auto tiff_size = [](thandle_t user_data) -> toff_t {
    auto* data = static_cast<read_context_t*>(user_data);

    auto current_offset = data->stream.tellg();
    data->stream.seekg(0, std::ios::end);
    auto end_offset = data->stream.tellg();
    data->stream.seekg(current_offset);

    return static_cast<toff_t>(end_offset);
  };

  static auto tiff_map = [](thandle_t /*user_data*/, tdata_t*, toff_t*) -> int {
    // NOP.
    return 0;
  };

  static auto tiff_unmap = [](thandle_t /*user_data*/, tdata_t, toff_t) {
    // NOP.
  };

  read_context_t context;
  // First open file stream.
  context.stream.open(filename.generic_string(),
                      std::ios_base::in | std::ios_base::binary);
  if (!context.stream.is_open())
  {
    log::error() << "Cannot open file " << filename;
    return false;
  }

  // Now open TIFF image from stream.
  auto* tiff =
    TIFFClientOpen("-", "r", &context, tiff_read, tiff_write, tiff_seek,
                   tiff_close, tiff_size, tiff_map, tiff_unmap);
  if (tiff == nullptr)
  {
    log::error() << "Cannot open TIFF file " << filename;
    context.stream.close();
    return false;
  }

  // Read all images stored in the TIFF file/stream.
  do
  {
    tiff_image image;

    if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &image.samples_per_pixel))
    {
      log::error()
        << filename
        << ": Missing required TIFF field 'TIFFTAG_SAMPLESPERPIXEL'.";
      return false;
    }

    {
      // TIFF extra samples are usually used for the number of alpha channels.
      std::uint16_t* extra_samples_info;
      TIFFGetFieldDefaulted(tiff, TIFFTAG_EXTRASAMPLES, &image.extra_samples,
                            &extra_samples_info);
    }

    if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &image.bits_per_sample))
    {
      log::error() << filename
                   << ": Missing required TIFF field 'TIFFTAG_BITSPERSAMPLE'.";
      return false;
    }
    if (image.bits_per_sample == 1)
    {
      log::error() << filename << ": Bilevel TIFF images are not supported.";
      return false;
    }
    else if (image.bits_per_sample != 8 && image.bits_per_sample != 16 &&
             image.bits_per_sample != 32)
    {
      log::error()
        << filename
        << ": TIFF images with bits per pixel other than 8, 16, or 32 "
           "are not supported.";
      return false;
    }

    if (std::uint16_t orientation;
        TIFFGetField(tiff, TIFFTAG_ORIENTATION, &orientation))
    {
      if (orientation != ORIENTATION_TOPLEFT)
      {
        /// ToDo: Implement image rotation and mirroring functions.
        log::error() << filename << ": Unsupported TIFF image orientation.";
        return false;
      }
    }
    else
    {
      log::warning()
        << "Missing TIFF field 'TIFFTAG_ORIENTATION'. Fallback to default.";
    }

    if (!TIFFGetField(tiff, TIFFTAG_SAMPLEFORMAT, &image.samples_format))
    {
      // Fallback to default.
      image.samples_format = tiff_samples_format::unsigned_int;
    }

    if (!TIFFGetField(tiff, TIFFTAG_COMPRESSION, &image.compression))
    {
      log::error() << filename
                   << ": Missing required TIFF field 'TIFFTAG_COMPRESSION'.";
      return false;
    }

    if (!TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &image.width))
    {
      log::error() << filename
                   << ": Missing required TIFF field 'TIFFTAG_IMAGEWIDTH'.";
      return false;
    }

    if (!TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &image.height))
    {
      log::error() << filename
                   << ": Missing required TIFF field 'TIFFTAG_IMAGELENGTH'.";
      return false;
    }

    if (!TIFFGetField(tiff, TIFFTAG_ROWSPERSTRIP, &image.rows_per_strip))
    {
      log::error() << filename
                   << ": Missing required TIFF field 'TIFFTAG_ROWSPERSTRIP'.";
      return false;
    }

    if (!TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &image.planar_config))
    {
      log::error() << filename
                   << ": Missing required TIFF field 'TIFFTAG_PLANARCONFIG'.";
      return false;
    }
    if (image.planar_config != tiff_planar_config::contiguous &&
        image.planar_config != tiff_planar_config::separate)
    {
      log::error() << filename << ": Unsupported planar configuration ("
                   << core::underlying_type_cast(image.planar_config) << ").";
      return 0;
    }

    if (!TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC, &image.photometric))
    {
      log::error() << filename
                   << ": Missing required TIFF field 'TIFFTAG_PHOTOMETRIC'.";
      return false;
    }

    switch (image.photometric)
    {
    case tiff_photometric::min_is_white:
    case tiff_photometric::min_is_black:
    case tiff_photometric::rgb:
    case tiff_photometric::separated:
    case tiff_photometric::cie_lab:
    case tiff_photometric::icc_lab:
      // NOP.
      break;

    case tiff_photometric::ycbcr:
      if (std::uint16_t subsampling_x = 1, subsampling_y = 1;
          !TIFFGetFieldDefaulted(tiff, TIFFTAG_YCBCRSUBSAMPLING, &subsampling_x,
                                 &subsampling_y) ||
          subsampling_x != 1 || subsampling_y != 1)
      {
        log::error() << filename << ": Subsampled images are not supported.";
        return false;
      }
      break;

    case tiff_photometric::log_luv:
      if (image.bits_per_sample != 16)
      {
        log::error()
          << filename
          << ": TIFF images with color space CIE Log2(L) (u',v') are "
             "required to have 16bits per sample.";
        return false;
      }
      break;

    case tiff_photometric::palette:
      log::error() << filename
                   << ": TIFF images with color palettes are note supported.";
      return false;

    default:
      log::error() << filename << ": Unsupported TIFF color space (photometric "
                   << core::underlying_type_cast(image.photometric) << ").";
      return false;
    }

    if (TIFFIsTiled(tiff))
    {
      log::error() << filename << ": We don't support tiled TIFF files, yet.";
      /// ToDo: Implement this case using TIFFReadEncodedTile().
      return false;
    }
    else
    {
      auto strip_size = TIFFStripSize(tiff);
#if 0
      log::info() << "TIFF file: " << filename;
      log::info() << "samples_per_pixel: " << image.samples_per_pixel;
      log::info() << "bits_per_sample: " << image.bits_per_sample;
      log::info() << "samples_format: " << image.samples_format;
      log::info() << "width: " << image.width;
      log::info() << "height: " << image.height;
      log::info() << "rows_per_strip: " << image.rows_per_strip;
      log::info() << "compression: " << image.compression;
      log::info() << "strip_size = " << strip_size;
#endif

      auto pixel_size = image.samples_per_pixel * image.bits_per_sample / 8u;
      image.pixel_data.resize(image.width * image.height * pixel_size);
      for (std::uint32_t y = 0, strip_id = 0; y < image.height;
           y += image.rows_per_strip, ++strip_id)
      {
        auto bytes_read = TIFFReadEncodedStrip(
          tiff, strip_id,
          image.pixel_data.data() + y * image.width * pixel_size,
          std::min(static_cast<tmsize_t>(image.height - y) * image.width *
                     pixel_size * image.rows_per_strip,
                   strip_size));
        if (bytes_read < 0)
          return false;
      }
    }

    if (!ignore_icc_profile)
    {
      if (std::pair<std::uint8_t*, std::uint32_t> icc_buffer = {nullptr, 0u};
          TIFFGetField(tiff, TIFFTAG_ICCPROFILE, &icc_buffer.second,
                       &icc_buffer.first) &&
          icc_buffer.first != nullptr && icc_buffer.second > 0)
      {
        image.icc_profile.reset(
          cmsOpenProfileFromMem(icc_buffer.first, icc_buffer.second));

        /// ToDo: Optionally print ICC profile info using
        /// PrintProfileInformation from
        /// https://github.com/mm2/Little-CMS/blob/lcms2.9/utils/common/vprf.c
        // // Print description found in the profile
        // if (Verbose && (image.icc_profile != nullptr))
        // {
        //   fprintf(stdout, "\n[Embedded profile]\n");
        //   PrintProfileInformation(image.icc_profile);
        // }
      }
      // Try to see if "colorimetric" tiff
      else if (float* primary_values = nullptr, *white_point_values = nullptr;
               TIFFGetField(tiff, TIFFTAG_PRIMARYCHROMATICITIES,
                            &primary_values) &&
               TIFFGetField(tiff, TIFFTAG_WHITEPOINT, &white_point_values) &&
               primary_values != nullptr && white_point_values != nullptr)
      {
        cmsCIExyYTRIPLE primaries;
        primaries.Red.x = static_cast<double>(primary_values[0]);
        primaries.Red.y = static_cast<double>(primary_values[1]);
        primaries.Red.Y = 1.0;
        primaries.Green.x = static_cast<double>(primary_values[2]);
        primaries.Green.y = static_cast<double>(primary_values[3]);
        primaries.Green.Y = 1.0;
        primaries.Blue.x = static_cast<double>(primary_values[4]);
        primaries.Blue.y = static_cast<double>(primary_values[5]);
        primaries.Blue.Y = 1.0;

        cmsCIExyY white_point;
        white_point.x = static_cast<double>(white_point_values[0]);
        white_point.y = static_cast<double>(white_point_values[1]);
        white_point.Y = 1.0;

        std::array<std::uint16_t*, 3> gm_rgb = {};
        TIFFGetFieldDefaulted(tiff, TIFFTAG_TRANSFERFUNCTION, &gm_rgb[0],
                              &gm_rgb[1], &gm_rgb[2]);
        std::array<cmsToneCurve*, 3> curves = {
          cmsBuildTabulatedToneCurve16(nullptr, 256, gm_rgb[0]),
          cmsBuildTabulatedToneCurve16(nullptr, 256, gm_rgb[1]),
          cmsBuildTabulatedToneCurve16(nullptr, 256, gm_rgb[2])};

        image.icc_profile.reset(cmsCreateRGBProfileTHR(
          nullptr, &white_point, &primaries, curves.data()));

        for (auto* curve : curves)
          cmsFreeToneCurve(curve);

        //        if (Verbose)
        //          fprintf(stdout, "\n[Colorimetric TIFF]\n");
      }
    }

    images.emplace_back(std::move(image));
  } while (TIFFReadDirectory(tiff) != 0);

  if (tiff != nullptr)
  {
    TIFFClose(tiff);
    tiff = nullptr;

    context.stream.close();
  }

  return true;
}

bool tiff_io::save(const std::filesystem::path& filename,
                   const std::vector<tiff_image>& images,
                   bool ignore_icc_profile)
{
  using namespace shift::rc::image_util;

  auto name = filename.generic_string();
  auto tiff = TIFFOpen(name.c_str(), "w");
  if (tiff == nullptr)
    return false;

  for (const auto& image : images)
  {
    TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, image.width);
    TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, image.height);
    TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, image.bits_per_sample);
    TIFFSetField(tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, image.samples_per_pixel);
    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, image.photometric);
    TIFFSetField(tiff, TIFFTAG_COMPRESSION, image.compression);
    TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, image.rows_per_strip);
    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, image.planar_config);

    switch (image.photometric)
    {
    case tiff_photometric::ycbcr:
    {
      std::uint16_t subsampling_x = 1;
      std::uint16_t subsampling_y = 1;
      TIFFSetField(tiff, TIFFTAG_YCBCRSUBSAMPLING, &subsampling_x,
                   &subsampling_y);
      break;
    }

    case tiff_photometric::log_luv:
      TIFFSetField(tiff, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_16BIT);
      break;

    default:
      // NOP.
      break;
    }

    if (!ignore_icc_profile)
    {
    }

    auto pixel_size = image.samples_per_pixel * image.bits_per_sample / 8u;
    // rows_per_strip = 1u;  // std::max(4096u / width * pixel_size, 1u);
    auto strip_size = image.width * pixel_size * image.rows_per_strip;
    for (std::uint32_t y = 0, strip_id = 0; y < image.height;
         y += image.rows_per_strip, ++strip_id)
    {
      TIFFWriteEncodedStrip(
        tiff, strip_id,
        const_cast<std::byte*>(image.pixel_data.data() +
                               y * image.width * pixel_size),
        std::min(static_cast<tmsize_t>(image.height - y) * image.width *
                   pixel_size * image.rows_per_strip,
                 static_cast<tmsize_t>(strip_size)));
    }
    TIFFWriteDirectory(tiff);
  }

  TIFFClose(tiff);
  return true;
}

bool tiff_io::override_icc_profile(tiff_image& image,
                                   tiff_icc_profile_category profile_category)
{
  switch (profile_category)
  {
  case tiff_icc_profile_category::linear:
  {
    const auto& icc_buffer = linear_icc();
    image.icc_profile.reset(cmsOpenProfileFromMem(
      icc_buffer.data(), static_cast<std::uint32_t>(icc_buffer.size())));
    image.icc_profile_category = profile_category;
    return true;
  }

  case tiff_icc_profile_category::srgb:
  {
    const auto& icc_buffer = srgb_icc();
    image.icc_profile.reset(cmsOpenProfileFromMem(
      icc_buffer.data(), static_cast<std::uint32_t>(icc_buffer.size())));
    image.icc_profile_category = profile_category;
    return true;
  }

  case tiff_icc_profile_category::custom:
    // This doesn't make sense.
    return false;
  }

  return false;
}

bool tiff_io::convert_to_linear(const tiff_image& source, tiff_image& target)
{
  //  cmsHPROFILE hIn, hOut, hProof, hInkLimit = nullptr;
  //  std::uint32_t wTarget;
  //  int bps = Width / 8;
  //  int nPlanes;

  /// ToDo: Make these constants configurable.
  // (values [0..1])
  static constexpr double observer_adaption_state = 1.0;
  static constexpr bool black_point_compensation = false;
  // Precalculates transform (0=Off, 1=Normal, 2=Hi-res, 3=LoRes)
  static constexpr std::uint32_t precalc_mode = 1;
  // Marks out-of-gamut colors on softproof
  static constexpr bool gamut_check = false;
  // static constexpr bool do_proofing = false;

  if (source.icc_profile == nullptr)
  {
    // The source image does not have an ICC profile. Please assign one with
    // override_icc_profile(source, tiff_icc_profile_category::linear); first.
    return false;
  }

  // Observer adaptation state (only meaningful on absolute colorimetric
  // intent)
  cmsSetAdaptationState(observer_adaption_state);

  //  if (EmbedProfile && cOutProf)
  //    DoEmbedProfile(out, cOutProf);

  std::uint32_t flags = 0;
  if (black_point_compensation)
    flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;

  switch (precalc_mode)
  {
  case 0:
    flags |= cmsFLAGS_NOOPTIMIZE;
    break;
  case 1:
    // No flag.
    break;
  case 2:
    flags |= cmsFLAGS_HIGHRESPRECALC;
    break;
  case 3:
    flags |= cmsFLAGS_LOWRESPRECALC;
    break;

  default:
    BOOST_ASSERT(false);
    return false;
  }

  if (gamut_check)
    flags |= cmsFLAGS_GAMUTCHECK;

  /// ToDo: Load linear color space profile.
  const auto& linear = linear_icc();
  target.icc_profile.reset(cmsOpenProfileFromMem(
    linear.data(), static_cast<std::uint32_t>(linear.size())));

  auto source_pixel_type = cms_get_input_pixel_type(source);

  BOOST_ASSERT(_cmsLCMScolorSpace(cmsGetColorSpace(source.icc_profile.get())) ==
               static_cast<int>(T_COLORSPACE(source_pixel_type)));

  int target_color_space =
    _cmsLCMScolorSpace(cmsGetColorSpace(target.icc_profile.get()));
  if (target_color_space < 0)
  {
    log::error() << "Illegal target color space.";
    return false;
  }

  auto target_pixel_type =
    FLOAT_SH(1) | COLORSPACE_SH(static_cast<unsigned int>(target_color_space)) |
    PLANAR_SH(T_PLANAR(source_pixel_type)) |
    CHANNELS_SH(T_CHANNELS(source_pixel_type)) | BYTES_SH(4);

  // WriteOutputTags(out, target_color_space, bps);
  // CopyOtherTags(in, out);

  //// Ink limit
  // if (InkLimit != 400.0 &&
  //    (target_color_space == PT_CMYK || target_color_space == PT_CMY))
  //{
  //  std::array<cmsHPROFILE, 3> hProfiles;

  //  hInkLimit =
  //  cmsCreateInkLimitingDeviceLink(cmsGetColorSpace(target.icc_profile),
  //                                             InkLimit);

  //  hProfiles[0] = source_profile;
  //  hProfiles[1] = target.icc_profile;
  //  hProfiles[2] = hInkLimit;

  //  transform = cmsCreateMultiprofileTransform(hProfiles, 3,
  //    source_pixel_type, target_pixel_type, Intent, flags);
  //  cmsCloseProfile(hInkLimit);
  //}
  // else
  //{
  auto transform = cmsCreateProofingTransform(
    source.icc_profile.get(), source_pixel_type, target.icc_profile.get(),
    target_pixel_type, nullptr, INTENT_PERCEPTUAL, INTENT_PERCEPTUAL, flags);
  //}

  if (transform == nullptr)
    return false;

  cmsDoTransform(transform, source.pixel_data.data(), target.pixel_data.data(),
                 source.width * source.height);

  return true;
}
}

#if 0
static cmsBool BlackWhiteCompensation = FALSE;
static cmsBool IgnoreEmbedded = FALSE;
static cmsBool EmbedProfile = FALSE;
static int Width = 8;
static cmsBool GamutCheck = FALSE;
static cmsBool lIsDeviceLink = FALSE;
static cmsBool StoreAsAlpha = FALSE;

static int Intent = INTENT_PERCEPTUAL;
static int ProofingIntent = INTENT_PERCEPTUAL;
static int PrecalcMode = 1;
static cmsFloat64Number InkLimit = 400;

static cmsFloat64Number ObserverAdaptationState =
  1.0;  // According ICC 4.3 this is the default

static const char* cInpProf = nullptr;
static const char* cOutProf = nullptr;
static const char* cProofing = nullptr;

// In TIFF, Lab is encoded in a different way, so let's use the plug-in
// capabilities of lcms2 to change the meaning of TYPE_Lab_8.

// * 0xffff / 0xff00 = (255 * 257) / (255 * 256) = 257 / 256
static int FromLabV2ToLabV4(int x)
{
  int a;

  a = ((x << 8) | x) >> 8;  // * 257 / 256
  if (a > 0xffff)
    return 0xffff;
  return a;
}

// * 0xf00 / 0xffff = * 256 / 257
static int FromLabV4ToLabV2(int x)
{
  return ((x << 8) + 0x80) / 257;
}

// Formatter for 8bit Lab TIFF (photometric 8)
static unsigned char* UnrollTIFFLab8(struct _cmstransform_struct* /*CMMcargo*/,
                                     cmsUInt16Number wIn[],
                                     cmsUInt8Number* accum,
                                     cmsUInt32Number /*Stride*/)
{
  wIn[0] = static_cast<cmsUInt16Number>(FromLabV2ToLabV4((accum[0]) << 8));
  wIn[1] = static_cast<cmsUInt16Number>(FromLabV2ToLabV4(
    ((accum[1] > 127) ? (accum[1] - 128) : (accum[1] + 128)) << 8));
  wIn[2] = static_cast<cmsUInt16Number>(FromLabV2ToLabV4(
    ((accum[2] > 127) ? (accum[2] - 128) : (accum[2] + 128)) << 8));

  return accum + 3;
}

// Formatter for 16bit Lab TIFF (photometric 8)
static unsigned char* UnrollTIFFLab16(struct _cmstransform_struct* /*CMMcargo*/,
                                      cmsUInt16Number wIn[],
                                      cmsUInt8Number* accum,
                                      cmsUInt32Number /*Stride*/)
{
  auto* accum16 = reinterpret_cast<cmsUInt16Number*>(accum);

  wIn[0] = static_cast<cmsUInt16Number>(FromLabV2ToLabV4(accum16[0]));
  wIn[1] = static_cast<cmsUInt16Number>(FromLabV2ToLabV4(
    ((accum16[1] > 0x7f00) ? (accum16[1] - 0x8000) : (accum16[1] + 0x8000))));
  wIn[2] = static_cast<cmsUInt16Number>(FromLabV2ToLabV4(
    ((accum16[2] > 0x7f00) ? (accum16[2] - 0x8000) : (accum16[2] + 0x8000))));

  return accum + 3 * sizeof(cmsUInt16Number);
}

static unsigned char* PackTIFFLab8(struct _cmstransform_struct* /*CMMcargo*/,
                                   cmsUInt16Number wOut[],
                                   cmsUInt8Number* output,
                                   cmsUInt32Number /*Stride*/)
{
  int a, b;

  *output++ =
    static_cast<cmsUInt8Number>(FromLabV4ToLabV2(wOut[0] + 0x0080) >> 8);

  a = (FromLabV4ToLabV2(wOut[1]) + 0x0080) >> 8;
  b = (FromLabV4ToLabV2(wOut[2]) + 0x0080) >> 8;

  *output++ = static_cast<cmsUInt8Number>((a < 128) ? (a + 128) : (a - 128));
  *output++ = static_cast<cmsUInt8Number>((b < 128) ? (b + 128) : (b - 128));

  return output;
}

static unsigned char* PackTIFFLab16(struct _cmstransform_struct* /*CMMcargo*/,
                                    cmsUInt16Number wOut[],
                                    cmsUInt8Number* output,
                                    cmsUInt32Number /*Stride*/)
{
  int a, b;
  auto* output16 = reinterpret_cast<cmsUInt16Number*>(output);

  *output16++ = static_cast<cmsUInt16Number>(FromLabV4ToLabV2(wOut[0]));

  a = FromLabV4ToLabV2(wOut[1]);
  b = FromLabV4ToLabV2(wOut[2]);

  *output16++ =
    static_cast<cmsUInt16Number>(((a < 0x7f00) ? (a + 0x8000) : (a - 0x8000)));
  *output16++ =
    static_cast<cmsUInt16Number>(((b < 0x7f00) ? (b + 0x8000) : (b - 0x8000)));

  return reinterpret_cast<cmsUInt8Number*>(output16);
}

static cmsFormatter TiffFormatterFactory(cmsUInt32Number Type,
                                         cmsFormatterDirection Dir,
                                         cmsUInt32Number dwFlags)
{
  cmsFormatter Result = {nullptr};
  int bps = T_BYTES(Type);
  int IsTiffSpecial = (Type >> 23) & 1;

  if (IsTiffSpecial && !(dwFlags & CMS_PACK_FLAGS_FLOAT))
  {
    if (Dir == cmsFormatterInput)
    {
      Result.Fmt16 = (bps == 1) ? UnrollTIFFLab8 : UnrollTIFFLab16;
    }
    else
      Result.Fmt16 = (bps == 1) ? PackTIFFLab8 : PackTIFFLab16;
  }

  return Result;
}

static cmsPluginFormatters TiffLabPlugin = {
  {cmsPluginMagicNumber, 2000, cmsPluginFormattersSig, nullptr},
  TiffFormatterFactory};

// Tile based transforms
static int TileBasedXform(cmsHTRANSFORM hXForm, TIFF* in, TIFF* out,
                          int nPlanes)
{
  tsize_t BufSizeIn = TIFFTileSize(in);
  tsize_t BufSizeOut = TIFFTileSize(out);
  unsigned char *BufferIn, *BufferOut;
  ttile_t i, TileCount = TIFFNumberOfTiles(in) / nPlanes;
  uint32 tw, tl;
  int PixelCount, j;

  TIFFGetFieldDefaulted(in, TIFFTAG_TILEWIDTH, &tw);
  TIFFGetFieldDefaulted(in, TIFFTAG_TILELENGTH, &tl);

  PixelCount = (int)tw * tl;

  BufferIn = (unsigned char*)_TIFFmalloc(BufSizeIn * nPlanes);
  if (!BufferIn)
    OutOfMem(BufSizeIn * nPlanes);

  BufferOut = (unsigned char*)_TIFFmalloc(BufSizeOut * nPlanes);
  if (!BufferOut)
    OutOfMem(BufSizeOut * nPlanes);

  for (i = 0; i < TileCount; i++)
  {

    for (j = 0; j < nPlanes; j++)
    {

      if (TIFFReadEncodedTile(in, i + (j * TileCount),
                              BufferIn + (j * BufSizeIn), BufSizeIn) < 0)
        goto cleanup;
    }

    cmsDoTransform(hXForm, BufferIn, BufferOut, PixelCount);

    for (j = 0; j < nPlanes; j++)
    {

      if (TIFFWriteEncodedTile(out, i + (j * TileCount),
                               BufferOut + (j * BufSizeOut), BufSizeOut) < 0)
        goto cleanup;
    }
  }

  _TIFFfree(BufferIn);
  _TIFFfree(BufferOut);
  return 1;

cleanup:

  _TIFFfree(BufferIn);
  _TIFFfree(BufferOut);
  return 0;
}

// Strip based transforms

static int StripBasedXform(cmsHTRANSFORM hXForm, TIFF* in, TIFF* out,
                           int nPlanes)
{
  tsize_t BufSizeIn = TIFFStripSize(in);
  tsize_t BufSizeOut = TIFFStripSize(out);
  unsigned char *BufferIn, *BufferOut;
  ttile_t i, StripCount = TIFFNumberOfStrips(in) / nPlanes;
  uint32 sw;
  uint32 sl;
  uint32 iml;
  int j;
  int PixelCount;

  TIFFGetFieldDefaulted(in, TIFFTAG_IMAGEWIDTH, &sw);
  TIFFGetFieldDefaulted(in, TIFFTAG_ROWSPERSTRIP, &sl);
  TIFFGetFieldDefaulted(in, TIFFTAG_IMAGELENGTH, &iml);

  // It is possible to get infinite rows per strip
  if (sl == 0 || sl > iml)
    sl = iml;  // One strip for whole image

  BufferIn = (unsigned char*)_TIFFmalloc(BufSizeIn * nPlanes);
  if (!BufferIn)
    OutOfMem(BufSizeIn * nPlanes);

  BufferOut = (unsigned char*)_TIFFmalloc(BufSizeOut * nPlanes);
  if (!BufferOut)
    OutOfMem(BufSizeOut * nPlanes);

  for (i = 0; i < StripCount; i++)
  {

    for (j = 0; j < nPlanes; j++)
    {

      if (TIFFReadEncodedStrip(in, i + (j * StripCount),
                               BufferIn + (j * BufSizeIn), BufSizeIn) < 0)
        goto cleanup;
    }

    PixelCount = (int)sw * (iml < sl ? iml : sl);
    iml -= sl;

    cmsDoTransform(hXForm, BufferIn, BufferOut, PixelCount);

    for (j = 0; j < nPlanes; j++)
    {
      if (TIFFWriteEncodedStrip(out, i + (j * StripCount),
                                BufferOut + j * BufSizeOut, BufSizeOut) < 0)
        goto cleanup;
    }
  }

  _TIFFfree(BufferIn);
  _TIFFfree(BufferOut);
  return 1;

cleanup:

  _TIFFfree(BufferIn);
  _TIFFfree(BufferOut);
  return 0;
}

// Creates minimum required tags
static void WriteOutputTags(TIFF* out, int Colorspace, int BytesPerSample)
{
  int BitsPerSample = (8 * BytesPerSample);
  int nChannels = ChanCountFromPixelType(Colorspace);

  uint16 Extra[] = {
    EXTRASAMPLE_UNASSALPHA, EXTRASAMPLE_UNASSALPHA, EXTRASAMPLE_UNASSALPHA,
    EXTRASAMPLE_UNASSALPHA, EXTRASAMPLE_UNASSALPHA, EXTRASAMPLE_UNASSALPHA,
    EXTRASAMPLE_UNASSALPHA, EXTRASAMPLE_UNASSALPHA, EXTRASAMPLE_UNASSALPHA,
    EXTRASAMPLE_UNASSALPHA, EXTRASAMPLE_UNASSALPHA};

  switch (Colorspace)
  {

  case PT_GRAY:
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, BitsPerSample);
    break;

  case PT_RGB:
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, BitsPerSample);
    break;

  case PT_CMY:
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(out, TIFFTAG_INKSET, 2);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, BitsPerSample);
    break;

  case PT_CMYK:
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
    TIFFSetField(out, TIFFTAG_INKSET, INKSET_CMYK);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, BitsPerSample);
    break;

  case PT_Lab:
    if (BitsPerSample == 16)
      TIFFSetField(out, TIFFTAG_PHOTOMETRIC, 9);
    else
      TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_CIELAB);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE,
                 BitsPerSample);  // Needed by TIFF Spec
    break;

    // Multi-ink separations
  case PT_MCH2:
  case PT_MCH3:
  case PT_MCH4:
  case PT_MCH5:
  case PT_MCH6:
  case PT_MCH7:
  case PT_MCH8:
  case PT_MCH9:
  case PT_MCH10:
  case PT_MCH11:
  case PT_MCH12:
  case PT_MCH13:
  case PT_MCH14:
  case PT_MCH15:

    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, nChannels);

    if (StoreAsAlpha && nChannels >= 4)
    {
      // CMYK plus extra alpha
      TIFFSetField(out, TIFFTAG_EXTRASAMPLES, nChannels - 4, Extra);
      TIFFSetField(out, TIFFTAG_INKSET, 1);
      TIFFSetField(out, TIFFTAG_NUMBEROFINKS, 4);
    }
    else
    {
      TIFFSetField(out, TIFFTAG_INKSET, 2);
      TIFFSetField(out, TIFFTAG_NUMBEROFINKS, nChannels);
    }

    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, BitsPerSample);
    break;

  default:
    FatalError("Unsupported output colorspace");
  }

  if (Width == 32)
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
}

// Copies a bunch of tages

static void CopyOtherTags(TIFF* in, TIFF* out)
{
#define CopyField(tag, v)        \
  if (TIFFGetField(in, tag, &v)) \
  TIFFSetField(out, tag, v)

  short shortv;
  uint32 ow, ol;
  cmsFloat32Number floatv;
  char* stringv;
  uint32 longv;

  CopyField(TIFFTAG_SUBFILETYPE, longv);

  TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &ow);
  TIFFGetField(in, TIFFTAG_IMAGELENGTH, &ol);

  TIFFSetField(out, TIFFTAG_IMAGEWIDTH, ow);
  TIFFSetField(out, TIFFTAG_IMAGELENGTH, ol);

  CopyField(TIFFTAG_PLANARCONFIG, shortv);
  CopyField(TIFFTAG_COMPRESSION, shortv);

  if (Width != 32)
    CopyField(TIFFTAG_PREDICTOR, shortv);

  CopyField(TIFFTAG_THRESHHOLDING, shortv);
  CopyField(TIFFTAG_FILLORDER, shortv);
  CopyField(TIFFTAG_ORIENTATION, shortv);
  CopyField(TIFFTAG_MINSAMPLEVALUE, shortv);
  CopyField(TIFFTAG_MAXSAMPLEVALUE, shortv);
  CopyField(TIFFTAG_XRESOLUTION, floatv);
  CopyField(TIFFTAG_YRESOLUTION, floatv);
  CopyField(TIFFTAG_RESOLUTIONUNIT, shortv);
  CopyField(TIFFTAG_ROWSPERSTRIP, longv);
  CopyField(TIFFTAG_XPOSITION, floatv);
  CopyField(TIFFTAG_YPOSITION, floatv);
  CopyField(TIFFTAG_IMAGEDEPTH, longv);
  CopyField(TIFFTAG_TILEDEPTH, longv);

  CopyField(TIFFTAG_TILEWIDTH, longv);
  CopyField(TIFFTAG_TILELENGTH, longv);

  CopyField(TIFFTAG_ARTIST, stringv);
  CopyField(TIFFTAG_IMAGEDESCRIPTION, stringv);
  CopyField(TIFFTAG_MAKE, stringv);
  CopyField(TIFFTAG_MODEL, stringv);

  CopyField(TIFFTAG_DATETIME, stringv);
  CopyField(TIFFTAG_HOSTCOMPUTER, stringv);
  CopyField(TIFFTAG_PAGENAME, stringv);
  CopyField(TIFFTAG_DOCUMENTNAME, stringv);
}

// A replacement for (the nonstandard) filelength

static void DoEmbedProfile(TIFF* Out, const char* ProfileFile)
{
  FILE* f;
  cmsInt32Number size;
  cmsUInt32Number EmbedLen;
  cmsUInt8Number* EmbedBuffer;

  f = fopen(ProfileFile, "rb");
  if (f == nullptr)
    return;

  size = cmsfilelength(f);
  if (size < 0)
    return;

  EmbedBuffer = (cmsUInt8Number*)malloc(size + 1);
  if (EmbedBuffer == nullptr)
  {
    OutOfMem(size + 1);
    return;
  }

  EmbedLen = (cmsUInt32Number)fread(EmbedBuffer, 1, (size_t)size, f);

  if (EmbedLen != size)
    FatalError("Cannot read %ld bytes to %s", size, ProfileFile);

  fclose(f);
  EmbedBuffer[EmbedLen] = 0;

  TIFFSetField(Out, TIFFTAG_ICCPROFILE, EmbedLen, EmbedBuffer);
  free(EmbedBuffer);
}

// Transform one image
static int TransformImage(TIFF* in, TIFF* out, const char* cDefInpProf)
{
  cmsHPROFILE hIn, hOut, hProof, hInkLimit = nullptr;
  cmsHTRANSFORM xform;
  cmsUInt32Number wInput, wOutput;
  int OutputColorSpace;
  int bps = Width / 8;
  cmsUInt32Number dwFlags = 0;
  int nPlanes;

  // Observer adaptation state (only meaningful on absolute colorimetric intent)

  cmsSetAdaptationState(ObserverAdaptationState);

  if (EmbedProfile && cOutProf)
    DoEmbedProfile(out, cOutProf);

  if (BlackWhiteCompensation)
    dwFlags |= cmsFLAGS_BLACKPOINTCOMPENSATION;

  switch (PrecalcMode)
  {

  case 0:
    dwFlags |= cmsFLAGS_NOOPTIMIZE;
    break;
  case 2:
    dwFlags |= cmsFLAGS_HIGHRESPRECALC;
    break;
  case 3:
    dwFlags |= cmsFLAGS_LOWRESPRECALC;
    break;
  case 1:
    break;

  default:
    FatalError("Unknown precalculation mode '%d'", PrecalcMode);
  }

  if (GamutCheck)
    dwFlags |= cmsFLAGS_GAMUTCHECK;

  hProof = nullptr;
  hOut = nullptr;

  if (lIsDeviceLink)
    hIn = cmsOpenProfileFromFile(cDefInpProf, "r");
  else
  {
    hIn = IgnoreEmbedded ? nullptr : GetTIFFProfile(in);

    if (hIn == nullptr)
      hIn = OpenStockProfile(nullptr, cDefInpProf);

    hOut = OpenStockProfile(nullptr, cOutProf);

    if (cProofing != nullptr)
    {

      hProof = OpenStockProfile(nullptr, cProofing);
      dwFlags |= cmsFLAGS_SOFTPROOFING;
    }
  }

  // Take input color space

  wInput = GetInputPixelType(in);

  // Assure both, input profile and input TIFF are on same colorspace

  if (_cmsLCMScolorSpace(cmsGetColorSpace(hIn)) != (int)T_COLORSPACE(wInput))
    FatalError("Input profile is not operating in proper color space");

  if (!lIsDeviceLink)
    OutputColorSpace = _cmsLCMScolorSpace(cmsGetColorSpace(hOut));
  else
    OutputColorSpace = _cmsLCMScolorSpace(cmsGetPCS(hIn));

  wOutput = ComputeOutputFormatDescriptor(wInput, OutputColorSpace, bps);

  WriteOutputTags(out, OutputColorSpace, bps);
  CopyOtherTags(in, out);

  // Ink limit
  if (InkLimit != 400.0 &&
      (OutputColorSpace == PT_CMYK || OutputColorSpace == PT_CMY))
  {

    cmsHPROFILE hProfiles[10];
    int nProfiles = 0;

    hInkLimit =
      cmsCreateInkLimitingDeviceLink(cmsGetColorSpace(hOut), InkLimit);

    hProfiles[nProfiles++] = hIn;
    if (hProof)
    {
      hProfiles[nProfiles++] = hProof;
      hProfiles[nProfiles++] = hProof;
    }

    hProfiles[nProfiles++] = hOut;
    hProfiles[nProfiles++] = hInkLimit;

    xform = cmsCreateMultiprofileTransform(hProfiles, nProfiles, wInput,
                                           wOutput, Intent, dwFlags);
  }
  else
  {

    xform = cmsCreateProofingTransform(hIn, wInput, hOut, wOutput, hProof,
                                       Intent, ProofingIntent, dwFlags);
  }

  cmsCloseProfile(hIn);
  cmsCloseProfile(hOut);

  if (hInkLimit)
    cmsCloseProfile(hInkLimit);
  if (hProof)
    cmsCloseProfile(hProof);

  if (xform == nullptr)
    return 0;

  // Planar stuff
  if (T_PLANAR(wInput))
    nPlanes = T_CHANNELS(wInput) + T_EXTRA(wInput);
  else
    nPlanes = 1;

  // Handle tile by tile or strip by strip
  if (TIFFIsTiled(in))
  {

    TileBasedXform(xform, in, out, nPlanes);
  }
  else
  {
    StripBasedXform(xform, in, out, nPlanes);
  }

  cmsDeleteTransform(xform);

  TIFFWriteDirectory(out);

  return 1;
}

// Print help
static void Help(int level)
{
  fprintf(
    stderr,
    "little cms ICC profile applier for TIFF - v6.2 [LittleCMS %2.2f]\n\n",
    LCMS_VERSION / 1000.0);
  fflush(stderr);

  switch (level)
  {

  default:
  case 0:

    fprintf(stderr, "usage: tificc [flags] input.tif output.tif\n");

    fprintf(stderr, "\nflags:\n\n");
    fprintf(stderr, "%cv - Verbose\n", SW);
    fprintf(stderr, "%ci<profile> - Input profile (defaults to sRGB)\n", SW);
    fprintf(stderr, "%co<profile> - Output profile (defaults to sRGB)\n", SW);
    fprintf(stderr, "%cl<profile> - Transform by device-link profile\n", SW);

    PrintRenderingIntents();

    fprintf(stderr, "%cb - Black point compensation\n", SW);
    fprintf(stderr, "%cd<0..1> - Observer adaptation state (abs.col. only)\n",
            SW);

    fprintf(stderr,
            "%cc<0,1,2,3> - Precalculates transform (0=Off, 1=Normal, "
            "2=Hi-res, 3=LoRes)\n",
            SW);
    fprintf(stderr, "\n");

    fprintf(stderr,
            "%cw<8,16,32> - Output depth. Use 32 for floating-point\n\n", SW);
    fprintf(stderr, "%ca - Handle channels > 4 as alpha\n", SW);

    fprintf(stderr, "%cn - Ignore embedded profile on input\n", SW);
    fprintf(stderr, "%ce - Embed destination profile\n", SW);
    fprintf(stderr, "\n");

    fprintf(stderr, "%cp<profile> - Soft proof profile\n", SW);
    fprintf(stderr, "%cm<n> - Soft proof intent\n", SW);
    fprintf(stderr, "%cg - Marks out-of-gamut colors on softproof\n", SW);

    fprintf(stderr, "\n");

    fprintf(stderr, "%ck<0..400> - Ink-limiting in %% (CMYK only)\n", SW);
    fprintf(stderr, "\n");
    fprintf(stderr, "%ch<0,1,2,3> - More help\n", SW);
    break;

  case 1:

    fprintf(stderr,
            "Examples:\n\n"
            "To color correct from scanner to sRGB:\n"
            "\ttificc %ciscanner.icm in.tif out.tif\n"
            "To convert from monitor1 to monitor2:\n"
            "\ttificc %cimon1.icm %comon2.icm in.tif out.tif\n"
            "To make a CMYK separation:\n"
            "\ttificc %coprinter.icm inrgb.tif outcmyk.tif\n"
            "To recover sRGB from a CMYK separation:\n"
            "\ttificc %ciprinter.icm incmyk.tif outrgb.tif\n"
            "To convert from CIELab TIFF to sRGB\n"
            "\ttificc %ci*Lab in.tif out.tif\n\n",
            SW, SW, SW, SW, SW, SW);
    break;

  case 2:
    PrintBuiltins();
    break;

  case 3:

    fprintf(stderr,
            "This program is intended to be a demo of the little cms\n"
            "engine. Both lcms and this program are freeware. You can\n"
            "obtain both in source code at http://www.littlecms.com\n"
            "For suggestions, comments, bug reports etc. send mail to\n"
            "info@littlecms.com\n\n");

    break;
  }

  fflush(stderr);
  exit(0);
}

// The toggles stuff

static void HandleSwitches(int argc, char* argv[])
{
  int s;

  while ((s = xgetopt(
            argc, argv,
            "aAeEbBw:W:nNvVGgh:H:i:I:o:O:P:p:t:T:c:C:l:L:M:m:K:k:S:s:D:d:")) !=
         EOF)
  {

    switch (s)
    {

    case 'a':
    case 'A':
      StoreAsAlpha = TRUE;
      break;
    case 'b':
    case 'B':
      BlackWhiteCompensation = TRUE;
      break;

    case 'c':
    case 'C':
      PrecalcMode = atoi(xoptarg);
      if (PrecalcMode < 0 || PrecalcMode > 3)
        FatalError("Unknown precalc mode '%d'", PrecalcMode);
      break;

    case 'd':
    case 'D':
      ObserverAdaptationState = atof(xoptarg);
      if (ObserverAdaptationState < 0 || ObserverAdaptationState > 1.0)
        log::warning() << "Adaptation state should be 0..1";
      break;

    case 'e':
    case 'E':
      EmbedProfile = TRUE;
      break;

    case 'g':
    case 'G':
      GamutCheck = TRUE;
      break;

    case 'v':
    case 'V':
      Verbose = TRUE;
      break;

    case 'i':
    case 'I':
      if (lIsDeviceLink)
        FatalError("Device-link already specified");

      cInpProf = xoptarg;
      break;

    case 'o':
    case 'O':
      if (lIsDeviceLink)
        FatalError("Device-link already specified");

      cOutProf = xoptarg;
      break;

    case 'l':
    case 'L':
      if (cInpProf != nullptr || cOutProf != nullptr)
        FatalError("input/output profiles already specified");

      cInpProf = xoptarg;
      lIsDeviceLink = TRUE;
      break;

    case 'p':
    case 'P':
      cProofing = xoptarg;
      break;

    case 't':
    case 'T':
      Intent = atoi(xoptarg);
      break;

    case 'm':
    case 'M':
      ProofingIntent = atoi(xoptarg);
      break;

    case 'N':
    case 'n':
      IgnoreEmbedded = TRUE;
      break;

    case 'W':
    case 'w':
      Width = atoi(xoptarg);
      if (Width != 8 && Width != 16 && Width != 32)
        FatalError("Only 8, 16 and 32 bps are supported");
      break;

    case 'k':
    case 'K':
      InkLimit = atof(xoptarg);
      if (InkLimit < 0.0 || InkLimit > 400.0)
        FatalError("Ink limit must be 0%%..400%%");
      break;

    case 'H':
    case 'h':
    {
      int a = atoi(xoptarg);
      Help(a);
    }
    break;

    default:

      FatalError("Unknown option - run without args to see valid ones");
    }
  }
}

// The main sink

int main(int argc, char* argv[])
{
  TIFF *in, *out;

  cmsPlugin(&TiffLabPlugin);

  InitUtils("tificc");

  HandleSwitches(argc, argv);

  if ((argc - xoptind) != 2)
  {

    Help(0);
  }

  in = TIFFOpen(argv[xoptind], "r");
  if (in == nullptr)
    FatalError("Unable to open '%s'", argv[xoptind]);

  out = TIFFOpen(argv[xoptind + 1], "w");

  if (out == nullptr)
  {

    TIFFClose(in);
    FatalError("Unable to write '%s'", argv[xoptind + 1]);
  }

  do
  {

    TransformImage(in, out, cInpProf);

  } while (TIFFReadDirectory(in));

  if (Verbose)
  {
    fprintf(stdout, "\n");
    fflush(stdout);
  }

  TIFFClose(in);
  TIFFClose(out);

  return 0;
}

// Virtual profiles are handled here.
cmsHPROFILE OpenStockProfile(cmsContext ContextID, const char* File)
{
  if (!File)
    return cmsCreate_sRGBProfileTHR(ContextID);

  if (cmsstrcasecmp(File, "*Lab2") == 0)
    return cmsCreateLab2ProfileTHR(ContextID, NULL);

  if (cmsstrcasecmp(File, "*Lab4") == 0)
    return cmsCreateLab4ProfileTHR(ContextID, NULL);

  if (cmsstrcasecmp(File, "*Lab") == 0)
    return cmsCreateLab4ProfileTHR(ContextID, NULL);

  if (cmsstrcasecmp(File, "*LabD65") == 0)
  {

    cmsCIExyY D65xyY;

    cmsWhitePointFromTemp(&D65xyY, 6504);
    return cmsCreateLab4ProfileTHR(ContextID, &D65xyY);
  }

  if (cmsstrcasecmp(File, "*XYZ") == 0)
    return cmsCreateXYZProfileTHR(ContextID);

  if (cmsstrcasecmp(File, "*Gray22") == 0)
  {

    cmsToneCurve* Curve = cmsBuildGamma(ContextID, 2.2);
    cmsHPROFILE hProfile =
      cmsCreateGrayProfileTHR(ContextID, cmsD50_xyY(), Curve);
    cmsFreeToneCurve(Curve);
    return hProfile;
  }

  if (cmsstrcasecmp(File, "*Gray30") == 0)
  {

    cmsToneCurve* Curve = cmsBuildGamma(ContextID, 3.0);
    cmsHPROFILE hProfile =
      cmsCreateGrayProfileTHR(ContextID, cmsD50_xyY(), Curve);
    cmsFreeToneCurve(Curve);
    return hProfile;
  }

  if (cmsstrcasecmp(File, "*srgb") == 0)
    return cmsCreate_sRGBProfileTHR(ContextID);

  if (cmsstrcasecmp(File, "*null") == 0)
    return cmsCreateNULLProfileTHR(ContextID);

  if (cmsstrcasecmp(File, "*Lin2222") == 0)
  {

    cmsToneCurve* Gamma = cmsBuildGamma(0, 2.2);
    cmsToneCurve* Gamma4[4];
    cmsHPROFILE hProfile;

    Gamma4[0] = Gamma4[1] = Gamma4[2] = Gamma4[3] = Gamma;
    hProfile = cmsCreateLinearizationDeviceLink(cmsSigCmykData, Gamma4);
    cmsFreeToneCurve(Gamma);
    return hProfile;
  }

  return cmsOpenProfileFromFileTHR(ContextID, File, "r");
}

#endif
