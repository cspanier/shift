#include "shift/rc/image/tiff_io.hpp"
#include "shift/rc/image/built_in_icc_profiles.hpp"
#include <shift/log/log.hpp>
#include <fstream>
#include <array>
#include <tiffio.h>
#define register
#include <lcms2_plugin.h>
#undef register

namespace shift::rc
{
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

static int cms_pixel_type_from_sample_count(int color_samples)
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
    return -1;
  }
}

static cmsUInt32Number cms_get_input_pixel_type(const tiff_image& image)
{
  int is_planar;
  switch (image.planar_config)
  {
  case PLANARCONFIG_CONTIG:
    is_planar = 0;
    break;
  case PLANARCONFIG_SEPARATE:
    is_planar = 1;
    break;
  default:
    BOOST_ASSERT(false);
    return 0;
  }

  // Reset planar configuration if the number of samples per pixel == 1.
  if (image.samples_per_pixel == 1)
    is_planar = 0;

  int extra_samples;
  int color_samples;
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

  int pixel_type = 0;
  int reverse = 0;
  int lab_tiff_special = 0;
  switch (image.photometric)
  {
  case PHOTOMETRIC_MINISWHITE:
    reverse = 1;
    [[fallthrough]];

  case PHOTOMETRIC_MINISBLACK:
    pixel_type = PT_GRAY;
    break;

  case PHOTOMETRIC_RGB:
    pixel_type = PT_RGB;
    break;

  case PHOTOMETRIC_SEPARATED:
    pixel_type = cms_pixel_type_from_sample_count(color_samples);
    break;

  case PHOTOMETRIC_YCBCR:
    pixel_type = PT_YCbCr;
    break;

  case PHOTOMETRIC_ICCLAB:
    pixel_type = PT_LabV2;
    break;

  case PHOTOMETRIC_CIELAB:
    pixel_type = PT_Lab;
    lab_tiff_special = 1;
    break;

  case PHOTOMETRIC_LOGLUV:
    pixel_type = PT_YUV;
    break;

  default:
    BOOST_ASSERT(false);
    return 0;
  }

  auto bytes_per_sample = image.bits_per_sample >> 3;
  int is_float = (bytes_per_sample == 0) || (bytes_per_sample == 4) ? 1 : 0;

  return (FLOAT_SH(is_float) | COLORSPACE_SH(pixel_type) |
          PLANAR_SH(is_planar) | EXTRA_SH(extra_samples) |
          CHANNELS_SH(color_samples) | BYTES_SH(bytes_per_sample) |
          FLAVOR_SH(reverse) | (lab_tiff_special << 23));
}

tiff_io::tiff_io()
{
  TIFFSetWarningHandlerExt(tiff_warning);
  TIFFSetErrorHandlerExt(tiff_error);

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
      log::error() << "Missing required TIFF field 'TIFFTAG_SAMPLESPERPIXEL'.";
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
      log::error() << "Missing required TIFF field 'TIFFTAG_BITSPERSAMPLE'.";
      return false;
    }
    BOOST_ASSERT(image.bits_per_sample == 8 || image.bits_per_sample == 16 ||
                 image.bits_per_sample == 32);
    if (image.bits_per_sample == 1)
    {
      log::error() << "Bilevel TIFF images are not supported.";
      return false;
    }

    if (std::uint16_t orientation;
        TIFFGetField(tiff, TIFFTAG_ORIENTATION, &orientation))
    {
      if (orientation != ORIENTATION_TOPLEFT)
      {
        /// ToDo: Implement image rotation and mirroring functions.
        log::error() << "Unsupported TIFF image orientation.";
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
      image.samples_format = SAMPLEFORMAT_UINT;
      log::warning()
        << "Missing TIFF field 'TIFFTAG_SAMPLEFORMAT'. Fallback to default.";
    }

    if (!TIFFGetField(tiff, TIFFTAG_COMPRESSION, &image.compression))
    {
      log::error() << "Missing required TIFF field 'TIFFTAG_COMPRESSION'.";
      return false;
    }

    if (!TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &image.width))
    {
      log::error() << "Missing required TIFF field 'TIFFTAG_IMAGEWIDTH'.";
      return false;
    }

    if (!TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &image.height))
    {
      log::error() << "Missing required TIFF field 'TIFFTAG_IMAGELENGTH'.";
      return false;
    }

    if (!TIFFGetField(tiff, TIFFTAG_ROWSPERSTRIP, &image.rows_per_strip))
    {
      log::error() << "Missing required TIFF field 'TIFFTAG_ROWSPERSTRIP'.";
      return false;
    }

    if (!TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &image.planar_config))
    {
      log::error() << "Missing required TIFF field 'TIFFTAG_PLANARCONFIG'.";
      return false;
    }
    if (image.planar_config != PLANARCONFIG_CONTIG &&
        image.planar_config != PLANARCONFIG_SEPARATE)
    {
      log::error() << "Unsupported planar configuration ("
                   << image.planar_config << ").";
      return 0;
    }

    if (!TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC, &image.photometric))
    {
      log::error() << "Missing required TIFF field 'TIFFTAG_PHOTOMETRIC'.";
      return false;
    }

    switch (image.photometric)
    {
    case PHOTOMETRIC_MINISWHITE:
    case PHOTOMETRIC_MINISBLACK:
    case PHOTOMETRIC_RGB:
    case PHOTOMETRIC_SEPARATED:
    case PHOTOMETRIC_CIELAB:
    case PHOTOMETRIC_ICCLAB:
      // NOP.
      break;

    case PHOTOMETRIC_YCBCR:
      if (std::uint16_t subsampling_x = 1, subsampling_y = 1;
          !TIFFGetFieldDefaulted(tiff, TIFFTAG_YCBCRSUBSAMPLING, &subsampling_x,
                                 &subsampling_y) ||
          subsampling_x != 1 || subsampling_y != 1)
      {
        log::error() << "Subsampled images are not supported.";
        return false;
      }
      break;

    case PHOTOMETRIC_LOGLUV:
      if (image.bits_per_sample != 16)
      {
        log::error() << "TIFF images with color space CIE Log2(L) (u',v') are "
                        "required to have 16bits per sample.";
        return false;
      }
      break;

    default:
      log::error() << "Unsupported TIFF color space (photometric "
                   << image.photometric << ").";
      return false;
    }

    if (TIFFIsTiled(tiff))
    {
      log::error() << "We don't support tiled TIFF files, yet.";
      /// ToDo: Implement this case using TIFFReadEncodedTile().
      return false;
    }
    else
    {
      auto strip_size = TIFFStripSize(tiff);
      log::info() << "samples_per_pixel: " << image.samples_per_pixel;
      log::info() << "bits_per_sample: " << image.bits_per_sample;
      log::info() << "samples_format: " << image.samples_format;
      log::info() << "width: " << image.width;
      log::info() << "height: " << image.height;
      log::info() << "rows_per_strip: " << image.rows_per_strip;
      log::info() << "compression: " << image.compression;
      log::info() << "strip_size = " << strip_size;

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
    case PHOTOMETRIC_YCBCR:
    {
      std::uint16_t subsampling_x = 1;
      std::uint16_t subsampling_y = 1;
      TIFFSetField(tiff, TIFFTAG_YCBCRSUBSAMPLING, &subsampling_x,
                   &subsampling_y);
    }

    case PHOTOMETRIC_LOGLUV:
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
    image.icc_profile.reset(
      cmsOpenProfileFromMem(icc_buffer.data(), icc_buffer.size()));
    image.icc_profile_category = profile_category;
    return true;
  }

  case tiff_icc_profile_category::srgb:
  {
    const auto& icc_buffer = srgb_icc();
    image.icc_profile.reset(
      cmsOpenProfileFromMem(icc_buffer.data(), icc_buffer.size()));
    image.icc_profile_category = profile_category;
    return true;
  }

  case tiff_icc_profile_category::custom:
    // This doesn't make sense.
    return false;
  }
}

bool tiff_io::convert_to_linear(const tiff_image& source, tiff_image& target)
{
  //  cmsHPROFILE hIn, hOut, hProof, hInkLimit = nullptr;
  //  cmsHTRANSFORM xform;
  //  cmsUInt32Number wTarget;
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
  //
  static constexpr bool do_proofing = false;

  if (source.icc_profile == nullptr)
  {
    // The source image does not have any ICC profile. Please assign one with
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

  cmsHPROFILE source_profile = source.icc_profile.get();

  return false;
}
}
