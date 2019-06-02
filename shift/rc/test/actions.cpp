#include <shift/rc/resource_compiler.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "utility.hpp"

#include <iostream>

using namespace shift::rc;
namespace fs = std::filesystem;

BOOST_AUTO_TEST_CASE(rc_action_no_rules)
{
  auto settings = create_working_folders();

  copy_files(settings.source_path, settings.input_path, "image32-rgb.*\\.tif");
  // Without a rules file we should not emit any jobs.
  run_rc(settings, 0, 0);
  // Run resource compiler a second time. We still expect no jobs to be
  // executed.
  run_rc(settings, 0, 0);
  remove_working_folders(settings);
}

BOOST_AUTO_TEST_CASE(rc_action_image_import)
{
  auto settings = create_working_folders();

  copy_files(settings.source_path, settings.input_path, "image32-rgb.*\\.tif");
  copy_files(settings.source_path / "action-image-import", settings.input_path,
             ".*");
  run_rc(settings, 5, 0);
  // Run resource compiler a second time. There should be nothing left to do.
  run_rc(settings, 0, 0);
  remove_working_folders(settings);
}
