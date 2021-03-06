#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/rc/image_util/tiff_io.hpp>
#include <shift/log/log_server.hpp>
#include <shift/core/at_exit_scope.hpp>
#include <squish.h>
#include "utility.hpp"

//#include <iostream>

// using namespace shift;
// namespace fs = std::filesystem;

// struct test_case_t
//{
//  const char* source_filename;
//  const char* destination_filename;
//  squish::flags_t flags;
//};

// const std::array<test_case_t, 21> test_cases = {
//  {{"squish-rgba8.tif", "squish-rgba8-bc1-low.tif",
//    squish::squish_flag::compression_bc1 |
//      squish::squish_flag::compressor_color_range_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba8.tif", "squish-rgba8-bc1-med.tif",
//    squish::squish_flag::compression_bc1 |
//      squish::squish_flag::compressor_color_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba8.tif", "squish-rgba8-bc1-high.tif",
//    squish::squish_flag::compression_bc1 |
//      squish::squish_flag::compressor_color_iterative_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba8.tif", "squish-rgba8-bc2-low.tif",
//    squish::squish_flag::compression_bc2 |
//      squish::squish_flag::compressor_color_range_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba8.tif", "squish-rgba8-bc2-med.tif",
//    squish::squish_flag::compression_bc2 |
//      squish::squish_flag::compressor_color_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba8.tif", "squish-rgba8-bc2-high.tif",
//    squish::squish_flag::compression_bc2 |
//      squish::squish_flag::compressor_color_iterative_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba8.tif", "squish-rgba8-bc3-low.tif",
//    squish::squish_flag::compression_bc3 |
//      squish::squish_flag::compressor_color_range_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba8.tif", "squish-rgba8-bc3-med.tif",
//    squish::squish_flag::compression_bc3 |
//      squish::squish_flag::compressor_color_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba8.tif", "squish-rgba8-bc3-high.tif",
//    squish::squish_flag::compression_bc3 |
//      squish::squish_flag::compressor_color_iterative_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba16.tif", "squish-rgba16-bc7-low.tif",
//    squish::squish_flag::compression_bc7 |
//      squish::squish_flag::compressor_color_range_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba16.tif", "squish-rgba16-bc7-med.tif",
//    squish::squish_flag::compression_bc7 |
//      squish::squish_flag::compressor_color_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"squish-rgba16.tif", "squish-rgba16-bc7-high.tif",
//    squish::squish_flag::compression_bc7 |
//      squish::squish_flag::compressor_color_iterative_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"broken_glass512-rgba8.tif", "broken_glass512-rgba8-bc1-low.tif",
//    squish::squish_flag::compression_bc1 |
//      squish::squish_flag::compressor_color_range_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"broken_glass512-rgba8.tif", "broken_glass512-rgba8-bc1-med.tif",
//    squish::squish_flag::compression_bc1 |
//      squish::squish_flag::compressor_color_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"broken_glass512-rgba8.tif", "broken_glass512-rgba8-bc1-high.tif",
//    squish::squish_flag::compression_bc1 |
//      squish::squish_flag::compressor_color_iterative_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"broken_glass512-rgba8.tif", "broken_glass512-rgba8-bc2-low.tif",
//    squish::squish_flag::compression_bc2 |
//      squish::squish_flag::compressor_color_range_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"broken_glass512-rgba8.tif", "broken_glass512-rgba8-bc2-med.tif",
//    squish::squish_flag::compression_bc2 |
//      squish::squish_flag::compressor_color_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"broken_glass512-rgba8.tif", "broken_glass512-rgba8-bc2-high.tif",
//    squish::squish_flag::compression_bc2 |
//      squish::squish_flag::compressor_color_iterative_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"broken_glass512-rgba8.tif", "broken_glass512-rgba8-bc3-low.tif",
//    squish::squish_flag::compression_bc3 |
//      squish::squish_flag::compressor_color_range_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"broken_glass512-rgba8.tif", "broken_glass512-rgba8-bc3-med.tif",
//    squish::squish_flag::compression_bc3 |
//      squish::squish_flag::compressor_color_cluster_fit |
//      squish::squish_flag::color_metric_perceptual},
//   {"broken_glass512-rgba8.tif", "broken_glass512-rgba8-bc3-high.tif",
//    squish::squish_flag::compression_bc3 |
//      squish::squish_flag::compressor_color_iterative_cluster_fit |
//      squish::squish_flag::color_metric_perceptual}}};

// BOOST_AUTO_TEST_CASE(rc_squish)
//{
//  core::at_exit_scope at_exit([]() { log::log_server::singleton_destroy(); });

//  auto& log_server = log::log_server::singleton_create();
//  log_server.add_console_sink(false, true, true, false);

//  for (const auto& test_case : test_cases)
//  {
//    auto source_filename = working_path() / "private" / "test" / "rc" /
//                           "source" / test_case.source_filename;
//    // std::cout << source_filename.string() << std::endl;
//    bool source_is_regular_file = fs::is_regular_file(source_filename);
//    BOOST_CHECK(source_is_regular_file);
//    if (!source_is_regular_file)
//      continue;

//    auto destination_filename = working_path() / "private" / "test" / "rc" /
//                                "squish" / test_case.destination_filename;

//    std::vector<rc::image_util::tiff_image> source_images;

//    rc::image_util::tiff_io io;
//    bool load_source = io.load(source_filename, source_images, false);
//    BOOST_CHECK(load_source);
//    if (!load_source)
//      continue;

//    std::vector<rc::image_util::tiff_image> destination_images;

//    for (auto& source_image : source_images)
//    {
//      BOOST_CHECK(source_image.samples_per_pixel == 4);
//      BOOST_CHECK(source_image.bits_per_sample >= 8);
//      BOOST_CHECK(source_image.width % 4 == 0);
//      BOOST_CHECK(source_image.height % 4 == 0);
//      if ((source_image.samples_per_pixel != 4) ||
//          (source_image.bits_per_sample < 8) || (source_image.width % 4 != 0)
//          || (source_image.height % 4 != 0))
//      {
//        continue;
//      }

//      auto destination_image = source_image;

//      std::uint32_t pixel_size =
//        source_image.samples_per_pixel * source_image.bits_per_sample / 8;
//      std::array<std::uint8_t, 4 * 4 * 8> source_block;
//      std::vector<std::byte> destination;
//      destination.resize(squish::storage_requirements(
//        source_image.width, source_image.height, test_case.flags));
//      for (std::uint32_t block_y = 0; block_y < source_image.height;
//           block_y += 4)
//      {
//        for (std::uint32_t block_x = 0; block_x < source_image.width;
//             block_x += 4)
//        {
//          std::uint32_t source_mask = 0;
//          for (std::uint32_t y = 0; y < 4; ++y)
//          {
//            auto absolute_y = block_y + y;
//            for (std::uint32_t x = 0; x < 4; ++x)
//            {
//              auto absolute_x = block_x + x;
//              if (absolute_x < source_image.width &&
//                  absolute_y < source_image.height)
//              {
//                std::memcpy(
//                  &source_block[(y * 4 + x) * pixel_size],
//                  &source_image.pixel_data[(absolute_y * source_image.width +
//                                            absolute_x) *
//                                           pixel_size],
//                  pixel_size);
//                source_mask |= 1 << (y * 4 + x);
//              }
//            }
//          }

//          squish::compress_masked(source_block.data(), source_mask,
//                                  destination.data(), test_case.flags);
//          squish::decompress(source_block.data(), destination.data(),
//                             test_case.flags);

//          for (std::uint32_t y = 0; y < 4; ++y)
//          {
//            auto absolute_y = block_y + y;
//            for (std::uint32_t x = 0; x < 4; ++x)
//            {
//              auto absolute_x = block_x + x;
//              if (absolute_x < destination_image.width &&
//                  absolute_y < destination_image.height)
//              {
//                std::memcpy(
//                  &destination_image
//                     .pixel_data[(absolute_y * destination_image.width +
//                                  absolute_x) *
//                                 pixel_size],
//                  &source_block[(y * 4 + x) * pixel_size], pixel_size);
//              }
//            }
//          }
//        }
//      }
//      destination_images.emplace_back(std::move(destination_image));
//    }

//    io.save(destination_filename, destination_images, false);
//  }
//}
