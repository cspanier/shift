#include <shift/rc/resource_compiler.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "utility.h"

using namespace shift::rc;

// Start resource compiler without any rules.
BOOST_AUTO_TEST_CASE(rc_rules_empty)
{
  auto settings = create_working_folders();
  run_rc(settings);
  // Run resource compiler a second time.
  run_rc(settings);
  remove_working_folders(settings);
}
