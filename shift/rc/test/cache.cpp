#include <shift/rc/resource_compiler.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "utility.hpp"
#include <thread>
#include <chrono>

using namespace shift::rc;

// BOOST_AUTO_TEST_CASE(rc_cache)
//{
//  std::error_code error_code;
//  auto settings = create_working_folders();

//  write_png_image(settings.input_path / "test_image1.png", 32, 32,
//  0xFF0077FF); write_png_image(settings.input_path / "test_image2.png", 32,
//  32, 0xFF77FF00); write_png_image(settings.input_path / "test_image3.png",
//  32, 32, 0xFFFF0077); write_text_file(settings.input_path / ".rc-rules.json",
//  R"({ "import": {
//    "pass": 1,
//    "action": "image-import",
//    "input": {
//      "image": "<input-path/><rule-path/>(test_image.*)\\.png$"
//    },
//    "output": {
//      "header": "<build-path/><rule-path/><image:1>.image_header",
//      "buffer":
//      "<output-path/><rule-path/><image:1>.lod_<lod-level>.image_buffer"
//    },
//    "options": {
//      "target-format": "rgba8_srgb",
//      "normalized": false
//    }
//  },
//  "global-cache": {
//    "pass": 2,
//    "action": "group-resources",
//    "input": {
//      "images": "<build-path/>(.*)\\.image_header$"
//    },
//    "group-by": [ 1 ],
//    "output": {
//      "group": "<output-path/>global.cache"
//    },
//    "options": {}
//  }
//}
//)");

//  auto check_files = [&]() {
//    BOOST_CHECK(
//      fs::file_size(settings.build_path / ".index.json", error_code) == 221);
//    BOOST_CHECK(!fs::exists(settings.input_path / "test_image1.png") ^
//                fs::exists(settings.build_path / "test_image1.image_header"));
//    BOOST_CHECK(!fs::exists(settings.input_path / "test_image2.png") ^
//                fs::exists(settings.build_path / "test_image2.image_header"));
//    BOOST_CHECK(!fs::exists(settings.input_path / "test_image3.png") ^
//                fs::exists(settings.build_path / "test_image3.image_header"));

//    BOOST_CHECK(
//      fs::file_size(settings.output_path / ".index.json", error_code) == 294);
//    BOOST_CHECK(
//      fs::file_size(settings.output_path / "global.cache", error_code) ==
//      494);
//    BOOST_CHECK(
//      !fs::exists(settings.input_path / "test_image1.png") ^
//      fs::exists(settings.output_path / "test_image1.lod_0.image_buffer"));
//    BOOST_CHECK(
//      !fs::exists(settings.input_path / "test_image2.png") ^
//      fs::exists(settings.output_path / "test_image2.lod_0.image_buffer"));
//    BOOST_CHECK(
//      !fs::exists(settings.input_path / "test_image3.png") ^
//      fs::exists(settings.output_path / "test_image3.lod_0.image_buffer"));
//  };

//  {
//    run_rc(settings, 4, 0);
//    check_files();

//    // Run resource compiler a second time. Everything should be cached and no
//    // jobs should run.
//    run_rc(settings, 0, 0);
//  }

//  {
//    // Remove an output file and check if it will be rebuilt on next rc
//    // invocation.
//    fs::remove(settings.output_path / "global.cache", error_code);
//    run_rc(settings, 1, 0);
//    check_files();

//    // Run resource compiler again. Everything should be cached.
//    run_rc(settings, 0, 0);
//  }

//  {
//    // Remove another output file and check if it will be rebuilt on next rc
//    // invocation.
//    fs::remove(settings.output_path / "test_image2.lod_0.image_buffer",
//               error_code);
//    run_rc(settings, 1, 0);
//    check_files();

//    // Run resource compiler again. Everything should be cached.
//    run_rc(settings, 0, 0);
//  }

//  {
//    // We need to sleep to make sure that the file that is rewritten below
//    gets
//    // a different timestamp. Otherwise the change might not be detected.
//    std::this_thread::sleep_for(std::chrono::seconds(2));

//    // Change content of a single image file.
//    write_png_image(settings.input_path / "test_image2.png", 32, 32,
//                    0xFF123456);
//    run_rc(settings, 2, 0);
//    check_files();

//    // Run resource compiler again. Everything should be cached.
//    run_rc(settings, 0, 0);
//  }

//  {
//    // Try to remove .index.json from output folder and check if it will be
//    // regenerated.
//    fs::remove(settings.output_path / ".index.json", error_code);
//    run_rc(settings, 0, 0);
//    check_files();
//  }

//  {
//    // Try to remove .index.json from build folder and check if it will be
//    // regenerated.
//    fs::remove(settings.build_path / ".index.json", error_code);
//    run_rc(settings, 0, 0);
//    check_files();
//  }

//  // remove_working_folders(settings);
//}
