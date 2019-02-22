#include <shift/rc/resource_compiler.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "utility.hpp"

#include <iostream>

using namespace shift::rc;
namespace fs = std::filesystem;

// Start resource compiler without any rules.
// BOOST_AUTO_TEST_CASE(rc_rules_empty)
//{
//  auto settings = create_working_folders();
//  {
//    // Without a rules file we should not emit any jobs.
//    auto [succeeded, failed] = run_rc(settings);
//    BOOST_CHECK_EQUAL(succeeded, 0);
//    BOOST_CHECK_EQUAL(failed, 0);
//  }
//  {
//    // Run resource compiler a second time. We still expect no jobs to be
//    // executed.
//    auto [succeeded, failed] = run_rc(settings);
//    BOOST_CHECK_EQUAL(succeeded, 0);
//    BOOST_CHECK_EQUAL(failed, 0);
//  }
//  remove_working_folders(settings);
//}

// BOOST_AUTO_TEST_CASE(rc_rules_update)
//{
//  auto settings = create_working_folders();

//  copy_files(settings.source_path, settings.input_path,
//  "image32_rgb.*\\.png"); copy_files(settings.source_path /
//  "action_image_import", settings.input_path,
//             ".*");
//  {
//    auto [succeeded, failed] = run_rc(settings);
//    BOOST_CHECK_EQUAL(succeeded, 7);
//    BOOST_CHECK_EQUAL(failed, 0);
//  }
//  {
//    // Run resource compiler a second time. Everything should be cached and no
//    // jobs should run.
//    auto [succeeded, failed] = run_rc(settings);
//    BOOST_CHECK_EQUAL(succeeded, 0);
//    BOOST_CHECK_EQUAL(failed, 0);
//  }
//  remove_working_folders(settings);
//}
