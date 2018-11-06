#include "utility.hpp"
#include <shift/rc/resource_compiler.hpp>
#include <shift/resource/repository.hpp>
#include <shift/task/async.hpp>
#include <shift/log/log_server.hpp>
#include <shift/core/at_exit_scope.hpp>
#include <shift/core/exception.hpp>
#include <shift/platform/environment.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <regex>

namespace fs = boost::filesystem;
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

std::tuple<std::size_t /*succeeded_job_count*/,
           std::size_t /*failed_job_count*/>
run_rc(const settings_t& settings)
{
  core::at_exit_scope at_exit([]() { log::log_server::singleton_destroy(); });

  auto& log_server = log::log_server::singleton_create();
  log_server.add_console_sink(false, true, true, false);

  resource::repository repository;
  repository.mount(settings.build_path, false);
  repository.mount(settings.output_path, false);

  std::size_t succeeded = 0;
  std::size_t failed = 0;

  auto primary_task = [&]() -> int {
    rc::resource_compiler compiler;
    compiler.input_path(settings.input_path);
    compiler.build_path(settings.build_path);
    compiler.output_path(settings.output_path);
    compiler.verbose(1);
    auto cache_filepath = settings.build_path / settings.cache_filename;
    if (fs::exists(cache_filepath))
      BOOST_CHECK(compiler.load_cache(cache_filepath));
    compiler.load_rules(".rc-rules.json");
    std::tie(succeeded, failed) = compiler.update();

    compiler.collect_garbage();
    compiler.save_cache(cache_filepath);
    return 0;
  };
  task::task_system{}.num_workers(1).start(primary_task).join();

  return {succeeded, failed};
}

void remove_working_folders(const settings_t& settings)
{
  fs::remove_all(settings.input_path);
  fs::remove_all(settings.build_path);
  fs::remove_all(settings.output_path);
}

void copy_files(const boost::filesystem::path& source_folder,
                const boost::filesystem::path& target_folder,
                std::string regex_pattern)
{
  std::regex regex(regex_pattern);

  boost::system::error_code error_code;
  fs::create_directories(target_folder, error_code);

  fs::directory_iterator end;
  for (fs::directory_iterator i{source_folder}; i != end; ++i)
  {
    if ((i->status().type() & fs::file_type::regular_file) != 0)
    {
      auto filename = i->path().filename();
      if (std::regex_match(filename.generic_string(), regex))
      {
        fs::copy_file(i->path(), target_folder / filename,
                      fs::copy_option::overwrite_if_exists);
      }
    }
  }
}
