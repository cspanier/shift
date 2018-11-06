#include <shift/rc/resource_compiler.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "utility.hpp"

using namespace shift::rc;

BOOST_AUTO_TEST_CASE(rc_cache)
{
  auto settings = create_working_folders();

  copy_files(settings.source_path, settings.input_path, "image32_rgb.*\\.png");
  copy_files(settings.source_path / "action_image_import", settings.input_path,
             ".*");
  {
    auto [succeeded, failed] = run_rc(settings);
    BOOST_CHECK_EQUAL(succeeded, 5);
    BOOST_CHECK_EQUAL(failed, 0);
  }
  {
    // Run resource compiler a second time. Everything should be cached and no
    // jobs should run.
    auto [succeeded, failed] = run_rc(settings);
    BOOST_CHECK_EQUAL(succeeded, 0);
    BOOST_CHECK_EQUAL(failed, 0);
  }
  // remove_working_folders(settings);
}
