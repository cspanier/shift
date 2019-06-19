#include "utility.hpp"
#include <shift/rc/resource_compiler.hpp>
#include <shift/rc/image_util/tiff_io.hpp>
#include <shift/resource_db/repository.hpp>
#include <shift/task/async.hpp>
#include <shift/log/log_server.hpp>
#include <shift/core/at_exit_scope.hpp>
#include <shift/core/exception.hpp>
#include <shift/platform/environment.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#define png_infopp_NULL nullptr
#define int_p_NULL nullptr
#include <boost/gil/extension/io/png.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <regex>
#include <fstream>

namespace fs = std::filesystem;
using namespace shift;

fs::path working_path()
{
  static fs::path result = platform::environment::executable_path()
                             .parent_path()
                             .parent_path()
                             .parent_path();
  return result;
}

settings_t create_working_folders()
{
  settings_t settings;
  settings.root_path = working_path() / "private" / "test" / "rc";
  settings.source_path = settings.root_path / "source";
  BOOST_CHECK(fs::exists(settings.source_path));
  BOOST_CHECK(fs::is_directory(settings.source_path));
  if (!fs::exists(settings.source_path) ||
      !fs::is_directory(settings.source_path))
  {
    BOOST_THROW_EXCEPTION(core::file_error() << core::file_name_info(
                            settings.source_path.generic_string()));
  }
  settings.input_path = settings.root_path / "input";
  settings.build_path = settings.root_path / "build";
  settings.output_path = settings.root_path / "output";
  remove_working_folders(settings);
  fs::create_directories(settings.input_path);
  fs::create_directories(settings.build_path);
  fs::create_directories(settings.output_path);
  return settings;
}

void run_rc(const settings_t& settings, std::size_t expect_succeeded,
            std::size_t expect_failed)
{
  core::at_exit_scope at_exit([]() { log::log_server::singleton_destroy(); });

  auto& log_server = log::log_server::singleton_create();
  log_server.add_console_sink(false, true, true, false);

  resource_db::repository repository;
  repository.mount(settings.build_path, false);
  repository.mount(settings.output_path, false);

  std::size_t succeeded = 0;
  std::size_t failed = 0;

  auto primary_task = [&]() -> int {
    rc::resource_compiler compiler;
    compiler.input_path(settings.input_path);
    compiler.build_path(settings.build_path);
    compiler.output_path(settings.output_path);
    compiler.verbose(3);
    auto cache_filepath = settings.build_path / settings.cache_filename;
    if (fs::exists(cache_filepath))
      BOOST_CHECK(compiler.load_cache(cache_filepath));
    compiler.load_rules(".rc-rules.json");
    std::tie(succeeded, failed) = compiler.update();

    compiler.collect_garbage();
    compiler.save_cache(cache_filepath);
    compiler.save_cache_graph(
      fs::path{cache_filepath}.replace_extension(".dot"));
    return 0;
  };
  task::task_system{}.num_workers(1).start(primary_task).join();

  BOOST_CHECK_EQUAL(succeeded, expect_succeeded);
  BOOST_CHECK_EQUAL(failed, expect_failed);
}

void remove_working_folders(const settings_t& settings)
{
  fs::remove_all(settings.input_path);
  fs::remove_all(settings.build_path);
  fs::remove_all(settings.output_path);
}

void copy_files(const std::filesystem::path& source_folder,
                const std::filesystem::path& target_folder,
                std::string regex_pattern)
{
  std::regex regex(regex_pattern);

  std::error_code error_code;
  fs::create_directories(target_folder, error_code);

  fs::directory_iterator end;
  for (fs::directory_iterator i{source_folder}; i != end; ++i)
  {
    if (i->status().type() == fs::file_type::regular)
    {
      auto filename = i->path().filename();
      if (std::regex_match(filename.generic_string(), regex))
      {
        fs::copy_file(i->path(), target_folder / filename,
                      fs::copy_options::overwrite_existing);
      }
    }
  }
}

void write_text_file(const std::filesystem::path& filename,
                     std::string_view content)
{
  std::ofstream file(filename.generic_string(),
                     std::ios_base::out | std::ios_base::trunc);
  if (file.is_open())
    file << content;
}

void write_png_image(const std::filesystem::path& filename, std::uint32_t width,
                     std::uint32_t height, std::uint32_t rgba)
{
  using namespace boost;
  gil::rgba8_image_t image(width, height);
  auto image_view = gil::view(image);
  gil::fill_pixels(image_view,
                   gil::rgba8_pixel_t(rgba & 0xFF, rgba >> 8 & 0xFF,
                                      rgba >> 16 & 0xFF, rgba >> 24 & 0xFF));
  gil::write_view(filename.generic_string(), image_view, gil::png_tag{});
}

void write_tiff_image(const std::filesystem::path& filename,
                      std::uint32_t width, std::uint32_t height,
                      std::uint32_t rgba)
{
  using namespace shift::rc::image_util;

  std::vector<tiff_image> images;
  auto& image = images.emplace_back();

  image.samples_per_pixel = 4;
  image.extra_samples = 0;
  image.bits_per_sample = 8;
  image.samples_format = tiff_samples_format::unsigned_int;
  image.compression = tiff_compression::zstd;
  image.width = width;
  image.height = height;
  image.rows_per_strip = height;
  image.photometric = tiff_photometric::rgb;
  image.planar_config = tiff_planar_config::contiguous;
  image.pixel_data.resize(image.width * image.height * image.samples_per_pixel *
                          image.bits_per_sample / 8);
  image.icc_profile_category = tiff_icc_profile_category::srgb;

  // Fill image with passed color.
  auto* destination = image.pixel_data.data();
  for (auto i = width * height; i > 0; --i)
  {
    std::memcpy(destination, &rgba, sizeof(rgba));
    destination += sizeof(rgba);
  }

  tiff_io::singleton_instance().save(filename, images, false);
}
