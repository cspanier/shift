#ifndef SHIFT_LOG_LOGFILESINK_HPP
#define SHIFT_LOG_LOGFILESINK_HPP

#include <chrono>
#include <fstream>
#include <string>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/filesystem/path.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/core/singleton.hpp"
#include "shift/log/log_sink.hpp"

namespace shift::log
{
/// A curses based text console with improved user interaction.
class log_file_sink
: public log_sink,
  public core::singleton<log_file_sink, core::create::on_stack>
{
public:
  /// Constructor.
  log_file_sink(boost::filesystem::path log_folder,
                std::string_view application_name, std::string_view suffix,
                bool enable_timestamp, bool enable_channel,
                bool enable_separator);

  /// Destructor.
  ~log_file_sink() override;

  /// @see log_sink::write.
  void write(std::chrono::system_clock::duration time_since_start, int channel,
             const channel_config& channel_config, channel_priority priority,
             std::string_view message, bool is_spam) override;

private:
  boost::filesystem::path _filename;
  std::ofstream _file;
  bool _enable_timestamp;
  bool _enable_channel;
  bool _enable_separator;
};
}

#endif
