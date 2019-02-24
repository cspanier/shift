#include "shift/rc/tiff/io.hpp"
#include <shift/log/log.hpp>
#include <fstream>
#include <tiffio.h>

namespace shift::rc::tiff
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

io::io()
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
    case COMPRESSION_ZSTD:
      log::info() << "  ZSTD";
      break;
    case COMPRESSION_WEBP:
      log::info() << "  WEBP";
      break;
    default:
      break;
    }
  }
  _TIFFfree(codecs);
}

bool io::load(const std::filesystem::path& filename, std::vector<image>& images)
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
    tiff::image image;
    if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &image.samples_per_pixel))
    {
      log::error() << "Missing required TIFF field 'TIFFTAG_SAMPLESPERPIXEL'.";
      return false;
    }

    if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &image.bits_per_sample))
    {
      log::error() << "Missing required TIFF field 'TIFFTAG_BITSPERSAMPLE'.";
      return false;
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
        tiff, strip_id, image.pixel_data.data() + y * image.width * pixel_size,
        std::min(static_cast<tmsize_t>(image.height - y) * image.width *
                   pixel_size * image.rows_per_strip,
                 strip_size));
      if (bytes_read < 0)
        return false;
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

bool io::save(const std::filesystem::path& filename,
              const std::vector<image>& images)
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
    TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tiff, TIFFTAG_COMPRESSION, image.compression);
    TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, image.rows_per_strip);

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
}
