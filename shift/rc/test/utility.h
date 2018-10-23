#ifndef TEST_SHIFT_RC_UTILITY_H
#define TEST_SHIFT_RC_UTILITY_H

#include <string>
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem.hpp>
#include <shift/core/boost_restore_warnings.h>

struct settings_t
{
  boost::filesystem::path root_path;
  boost::filesystem::path input_path;
  boost::filesystem::path build_path;
  boost::filesystem::path output_path;
  std::string cache_filename = ".rc-cache.json";
  std::string rules_filename = ".rc-rules.json";
};

boost::filesystem::path working_path();

settings_t create_working_folders();

void run_rc(const settings_t& settings);

void remove_working_folders(const settings_t& settings);

#endif
