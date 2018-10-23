#include "utility.h"
#include <shift/rc/resource_compiler.h>
#include <shift/log/log_server.h>
#include <shift/core/at_exit_scope.h>
#include <shift/platform/environment.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

namespace fs = boost::filesystem;
using namespace shift;

fs::path working_path()
{
  return fs::path(platform::environment::executable_path())
    .parent_path()
    .parent_path()
    .parent_path();
}

settings_t create_working_folders()
{
  settings_t result;
  result.root_path = working_path() / "private" / "test" / "rc";
  result.input_path = result.root_path / "input";
  result.build_path = result.root_path / "build";
  result.output_path = result.root_path / "output";
  fs::create_directories(result.input_path);
  fs::create_directories(result.build_path);
  fs::create_directories(result.output_path);
  return result;
}

void run_rc(const settings_t& settings)
{
  auto& log_server = log::log_server::singleton_create();
  core::at_exit_scope at_exit([]() { log::log_server::singleton_destroy(); });

  log_server.add_console_sink(false, true, true, false);

  rc::resource_compiler compiler;
  compiler.input_path(settings.input_path);
  compiler.build_path(settings.build_path);
  compiler.output_path(settings.output_path);
  compiler.verbose(1);
  auto cache_filepath = settings.build_path / settings.cache_filename;
  if (fs::exists(cache_filepath))
    BOOST_CHECK(compiler.load_cache(cache_filepath));
  compiler.load_rules(".rc-rules.json");
  compiler.update();

  compiler.collect_garbage();
  compiler.save_cache(cache_filepath);
}

void remove_working_folders(const settings_t& settings)
{
  fs::remove_all(settings.root_path);
}
