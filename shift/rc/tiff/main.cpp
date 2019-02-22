#include <shift/application/launcher.hpp>
#include <shift/task/launcher.hpp>
#include <shift/log/log.hpp>
#include <shift/core/at_exit_scope.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <compressonator/Compressonator.h>
#include <tiffio.h>

namespace shift::rc
{
struct program_options
{
  static std::vector<std::filesystem::path> input_paths;
};

std::vector<std::filesystem::path> program_options::input_paths;

template <typename NextModule>
class launcher : public NextModule, public program_options
{
public:
  using base_t = NextModule;

  launcher(int argc, char* argv[]) : base_t(argc, argv)
  {
    namespace opt = boost::program_options;

    base_t::_visible_options.add_options()(
      "input,i", opt::value(&input_paths)->composing()->required(),
      "Path to an input TIFF file.");
  }
};

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
};

struct tiff_io
{
  ///
  bool load(const std::filesystem::path& filename,
            std::vector<tiff_image>& images);

  ///
  bool save(const std::filesystem::path& filename,
            const std::vector<tiff_image>& images);
};

bool tiff_io::load(const std::filesystem::path& filename,
                   std::vector<tiff_image>& images)
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

bool tiff_io::save(const std::filesystem::path& filename,
                   const std::vector<tiff_image>& images)
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

///
class application : public program_options
{
public:
  /// Constructor.
  application();

  /// Main application routine.
  int run();
};

bool compression_callback(float progress, std::size_t /*user1*/,
                          std::size_t /*user2*/)
{
  log::info() << "Compression progress = " << progress;
  return false;
}

cmp_format tiff_to_cmp_format(const tiff_image& image)
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

application::application() = default;

int application::run()
{
  namespace fs = std::filesystem;

  for (const auto& input_path : input_paths)
  {
    log::info() << "Loading file " << input_path;

    tiff_io io;
    std::vector<tiff_image> images;
    if (!io.load(input_path, images))
    {
      log::error() << "Failed loading image.";
      return EXIT_FAILURE;
    }

    for (auto& image : images)
    {
      // Compress image using Compressonator.
      CMP_Texture source;
      source.width = image.width;
      source.height = image.height;
      source.pitch = 0;
      source.format = tiff_to_cmp_format(image);
      /// ToDo: Why 32bit only?
      source.data_size = static_cast<std::uint32_t>(image.pixel_data.size());
      /// ToDo: Why uint8_t?
      source.data = reinterpret_cast<std::uint8_t*>(image.pixel_data.data());

      CMP_Texture compressed;
      compressed.width = image.width;
      compressed.height = image.height;
      compressed.pitch = 0;
      compressed.format = cmp_format::BC1;
      std::vector<std::byte> compressed_data;
      compressed_data.resize(CMP_CalculateBufferSize(&compressed));
      /// ToDo: Why 32bit only?
      compressed.data_size = static_cast<std::uint32_t>(compressed_data.size());
      /// ToDo: Why uint8_t?
      compressed.data = reinterpret_cast<std::uint8_t*>(compressed_data.data());

      CMP_CompressOptions options{};
      options.dwSize = sizeof(options);
      options.fquality = 0.05f;
      options.dwnumThreads = 16;

      CMP_ERROR status = CMP_ConvertTexture(&source, &compressed, &options,
                                            &compression_callback, 0, 0);
      if (status == CMP_OK)
      {
        image.samples_per_pixel = 4;
        auto pixel_size = image.samples_per_pixel * image.bits_per_sample / 8u;
        image.pixel_data.resize(image.width * image.height * pixel_size);

        source.format = cmp_format::RGBA_8888;
        source.data_size = static_cast<std::uint32_t>(image.pixel_data.size());
        source.data = reinterpret_cast<std::uint8_t*>(image.pixel_data.data());

        status = CMP_ConvertTexture(&compressed, &source, &options,
                                    &compression_callback, 0, 0);
      }

      image.compression = COMPRESSION_ZSTD;
    }

    io.save(fs::path{input_path}.replace_extension(".copy.tif"), images);
  }

  return EXIT_SUCCESS;
}
}

int main(int argc, char* argv[])
{
  using namespace shift;

  application::launcher_t<rc::launcher> launcher(argc, argv);
  return launcher.execute([]() { return rc::application{}.run(); });
}
