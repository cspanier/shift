#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/rc/image/tiff_io.hpp>
#include <shift/log/log_server.hpp>
#include <shift/core/at_exit_scope.hpp>
#include <squish.h>
#include "utility.hpp"

#include <iostream>

using namespace shift;
using namespace shift::rc;
namespace fs = std::filesystem;

struct test_case_t
{
  const char* source_filename;
  const char* destination_filename;
  squish::flags_t flags;
};

const std::array<test_case_t, 12> test_cases = {
  {{"rgba8.tif", "rgba8_bc1_low.tif",
    squish::squish_flag::compression_bc1 |
      squish::squish_flag::compressor_color_range_fit |
      squish::squish_flag::color_metric_perceptual},
   {"rgba8.tif", "rgba8_bc1_med.tif",
    squish::squish_flag::compression_bc1 |
      squish::squish_flag::compressor_color_cluster_fit |
      squish::squish_flag::color_metric_perceptual},
   {"rgba8.tif", "rgba8_bc1_high.tif",
    squish::squish_flag::compression_bc1 |
      squish::squish_flag::compressor_color_iterative_cluster_fit |
      squish::squish_flag::color_metric_perceptual},
   {"rgba8.tif", "rgba8_bc2_low.tif",
    squish::squish_flag::compression_bc2 |
      squish::squish_flag::compressor_color_range_fit |
      squish::squish_flag::color_metric_perceptual},
   {"rgba8.tif", "rgba8_bc2_med.tif",
    squish::squish_flag::compression_bc2 |
      squish::squish_flag::compressor_color_cluster_fit |
      squish::squish_flag::color_metric_perceptual},
   {"rgba8.tif", "rgba8_bc2_high.tif",
    squish::squish_flag::compression_bc2 |
      squish::squish_flag::compressor_color_iterative_cluster_fit |
      squish::squish_flag::color_metric_perceptual},
   {"rgba8.tif", "rgba8_bc3_low.tif",
    squish::squish_flag::compression_bc3 |
      squish::squish_flag::compressor_color_range_fit |
      squish::squish_flag::color_metric_perceptual},
   {"rgba8.tif", "rgba8_bc3_med.tif",
    squish::squish_flag::compression_bc3 |
      squish::squish_flag::compressor_color_cluster_fit |
      squish::squish_flag::color_metric_perceptual},
   {"rgba8.tif", "rgba8_bc3_high.tif",
    squish::squish_flag::compression_bc3 |
      squish::squish_flag::compressor_color_iterative_cluster_fit |
      squish::squish_flag::color_metric_perceptual},
   //   {"rgba16.tif", "rgba16_bc7_low.tif",
   //    squish::squish_flag::compression_bc7 |
   //    squish::squish_flag::compressor_color_range_fit |
   //      squish::squish_flag::color_metric_perceptual},
   //   {"rgba16.tif", "rgba16_bc7_med.tif",
   //    squish::squish_flag::compression_bc7 |
   //    squish::squish_flag::compressor_color_cluster_fit |
   //      squish::squish_flag::color_metric_perceptual},
   //   {"rgba16.tif", "rgba16_bc7_high.tif",
   //    squish::squish_flag::compression_bc7 |
   //    squish::squish_flag::compressor_color_iterative_cluster_fit |
   //      squish::squish_flag::color_metric_perceptual},
   //   {"broken_glass512_rgba8.tif", "broken_glass512_rgba8_bc1_low.tif",
   //    squish::squish_flag::compression_bc1 |
   //    squish::squish_flag::compressor_color_range_fit |
   //      squish::squish_flag::color_metric_perceptual},
   //   {"broken_glass512_rgba8.tif", "broken_glass512_rgba8_bc1_med.tif",
   //    squish::squish_flag::compression_bc1 |
   //    squish::squish_flag::compressor_color_cluster_fit |
   //      squish::squish_flag::color_metric_perceptual},
   //   {"broken_glass512_rgba8.tif", "broken_glass512_rgba8_bc1_high.tif",
   //    squish::squish_flag::compression_bc1 |
   //    squish::squish_flag::compressor_color_iterative_cluster_fit |
   //      squish::squish_flag::color_metric_perceptual},
   //   {"broken_glass512_rgba8.tif", "broken_glass512_rgba8_bc2_low.tif",
   //    squish::squish_flag::compression_bc2 |
   //    squish::squish_flag::compressor_color_range_fit |
   //      squish::squish_flag::color_metric_perceptual},
   //   {"broken_glass512_rgba8.tif", "broken_glass512_rgba8_bc2_med.tif",
   //    squish::squish_flag::compression_bc2 |
   //    squish::squish_flag::compressor_color_cluster_fit |
   //      squish::squish_flag::color_metric_perceptual},
   //   {"broken_glass512_rgba8.tif", "broken_glass512_rgba8_bc2_high.tif",
   //    squish::squish_flag::compression_bc2 |
   //    squish::squish_flag::compressor_color_iterative_cluster_fit |
   //      squish::squish_flag::color_metric_perceptual},
   {"broken_glass512_rgba8.tif", "broken_glass512_rgba8_bc3_low.tif",
    squish::squish_flag::compression_bc3 |
      squish::squish_flag::compressor_color_range_fit |
      squish::squish_flag::color_metric_perceptual},
   {"broken_glass512_rgba8.tif", "broken_glass512_rgba8_bc3_med.tif",
    squish::squish_flag::compression_bc3 |
      squish::squish_flag::compressor_color_cluster_fit |
      squish::squish_flag::color_metric_perceptual},
   {"broken_glass512_rgba8.tif", "broken_glass512_rgba8_bc3_high.tif",
    squish::squish_flag::compression_bc3 |
      squish::squish_flag::compressor_color_iterative_cluster_fit |
      squish::squish_flag::color_metric_perceptual}}};

BOOST_AUTO_TEST_CASE(rc_squish)
{
  core::at_exit_scope at_exit([]() { log::log_server::singleton_destroy(); });

  auto& log_server = log::log_server::singleton_create();
  log_server.add_console_sink(false, true, true, false);

  for (const auto& test_case : test_cases)
  {
    auto source_filename = working_path() / "private" / "test" / "rc" /
                           "source" / test_case.source_filename;
    // std::cout << source_filename.string() << std::endl;
    bool source_is_regular_file = fs::is_regular_file(source_filename);
    BOOST_CHECK(source_is_regular_file);
    if (!source_is_regular_file)
      continue;

    auto destination_filename = working_path() / "private" / "test" / "rc" /
                                "squish" / test_case.destination_filename;

    std::vector<tiff_image> source_images;

    tiff_io io;
    bool load_source = io.load(source_filename, source_images, false);
    BOOST_CHECK(load_source);
    if (!load_source)
      continue;

    std::vector<tiff_image> destination_images;

    for (auto& source_image : source_images)
    {
      BOOST_CHECK(source_image.samples_per_pixel == 4);
      BOOST_CHECK(source_image.bits_per_sample == 8);
      BOOST_CHECK(source_image.width % 4 == 0);
      BOOST_CHECK(source_image.height % 4 == 0);
      if ((source_image.samples_per_pixel != 4) ||
          (source_image.bits_per_sample != 8) ||
          (source_image.width % 4 != 0) || (source_image.height % 4 != 0))
      {
        continue;
      }

      auto destination_image = source_image;

      std::array<std::uint8_t, 4 * 4> source_block;
      std::uint32_t source_mask = 0xFFFFFFFF;
      std::vector<std::byte> destination;
      destination.resize(squish::storage_requirements(
        source_image.width, source_image.height, test_case.flags));
      for (std::uint32_t block_y = 0; block_y < source_image.height;
           block_y += 4)
      {
        for (std::uint32_t block_x = 0; block_x < source_image.width;
             block_x += 4)
        {
          for (std::uint32_t y = 0; y < 4; ++y)
          {
            auto absolute_y = block_y + y;
            for (std::uint32_t x = 0; x < 4; ++x)
            {
              auto absolute_x = block_x + x;
              if (absolute_x < source_image.width &&
                  absolute_y < source_image.height)
              {
                std::uint32_t pixel;
                std::memcpy(
                  &pixel,
                  &source_image.pixel_data
                     [(absolute_y * source_image.width + absolute_x) * 4],
                  sizeof(pixel));
                // pixel = (pixel << 24) | (pixel << 8 & 0x00FF0000) |
                //         (pixel >> 8 & 0x0000FF00) | (pixel >> 24);
                pixel = (pixel & 0xFF00FF00) | (pixel << 16 & 0x00FF0000) |
                        (pixel >> 16 & 0x000000FF);
                std::memcpy(&source_block[(y * 4 + x) * 4], &pixel,
                            sizeof(pixel));
                source_mask |= 1 << (y * 4 + x);
              }
            }
          }

          squish::compress_masked(source_block.data(), source_mask,
                                  destination.data(), test_case.flags);
          squish::decompress(source_block.data(), destination.data(),
                             test_case.flags);

          for (std::uint32_t y = 0; y < 4; ++y)
          {
            auto absolute_y = block_y + y;
            for (std::uint32_t x = 0; x < 4; ++x)
            {
              auto absolute_x = block_x + x;
              if (absolute_x < destination_image.width &&
                  absolute_y < destination_image.height)
              {
                std::uint32_t pixel;
                std::memcpy(&pixel, &source_block[(y * 4 + x) * 4],
                            sizeof(pixel));
                // pixel = (pixel << 24) | (pixel << 8 & 0x00FF0000) |
                //         (pixel >> 8 & 0x0000FF00) | (pixel >> 24);
                pixel = (pixel & 0xFF00FF00) | (pixel << 16 & 0x00FF0000) |
                        (pixel >> 16 & 0x000000FF);
                std::memcpy(
                  &destination_image.pixel_data
                     [(absolute_y * destination_image.width + absolute_x) * 4],
                  &pixel, sizeof(pixel));
              }
            }
          }
        }
      }
      destination_images.emplace_back(std::move(destination_image));
    }

    io.save(destination_filename, destination_images, false);
  }
}
