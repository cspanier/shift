#include "shift/log/log_file_sink.hpp"
#include "shift/log/log.hpp"
#include <shift/platform/assert.hpp>
#include <cwchar>
#include <chrono>
#include <iomanip>
#include <string>
#include <memory>
#include <ctime>
#include <cstring>
#include <ios>

namespace shift::log
{
log_file_sink::log_file_sink(std::filesystem::path log_folder,
                             std::string_view application_name,
                             std::string_view suffix, bool enable_timestamp,
                             bool enable_channel, bool enable_separator)
: _enable_timestamp(enable_timestamp),
  _enable_channel(enable_channel),
  _enable_separator(enable_separator)
{
  namespace fs = std::filesystem;
  if (!fs::exists(log_folder))
  {
    std::error_code error;
    info() << "Creating missing log file folder " << log_folder;
    fs::create_directory(log_folder, error);
    if (error)
      log_folder = fs::current_path();
  }
  else if (!fs::is_directory(log_folder))
  {
    BOOST_ASSERT(false);
    log_folder = fs::current_path();
  }

  auto now =
    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  tm utc = {};
  std::memset(&utc, 0, sizeof(utc));
#if defined(SHIFT_PLATFORM_LINUX)
  // Ignore any potential error of gmtime_r.
  gmtime_r(&now, &utc);
#elif defined(SHIFT_PLATFORM_WINDOWS)
  // Ignore any potential error of gmtime_s.
  gmtime_s(&utc, &now);
#endif

  std::stringstream log_filename;
  log_filename << application_name << "_" << std::setfill('0') << std::setw(4)
               << (1900 + utc.tm_year) << std::setw(2) << (utc.tm_mon + 1)
               << std::setw(2) << utc.tm_mday << "_" << std::setw(2)
               << utc.tm_hour << std::setw(2) << utc.tm_min << std::setw(2)
               << utc.tm_sec;
  if (!suffix.empty())
    log_filename << "_" << suffix;
  log_filename << ".log";
  _filename = log_folder / log_filename.str();
  info() << "Using log file " << _filename;

  _file.open(_filename.generic_string(),
             std::ios_base::out | std::ios_base::trunc);
}

log_file_sink::~log_file_sink()
{
  _file.close();
}

void log_file_sink::write(std::chrono::system_clock::duration time_since_start,
                          int channel, const channel_config& channel_config,
                          channel_priority /*priority*/,
                          std::string_view message, bool /*is_spam*/)
{
  if (channel == status_channel || !_file.is_open())
    return;

  if (_enable_timestamp)
  {
    _file << std::setw(6) << std::setfill('0')
          << (std::chrono::duration_cast<std::chrono::seconds>(time_since_start)
                .count() %
              1000000)
          << ":" << std::setw(3) << std::setfill('0')
          << (std::chrono::duration_cast<std::chrono::milliseconds>(
                time_since_start)
                .count() %
              1000)
          << " ";
  }
  if (_enable_channel)
  {
    _file << channel_config.channel_name
          << std::string(channel_config.indentation, ' ');
  }
  if (_enable_separator)
    _file << ": ";
  _file << message << std::endl;
}
}
