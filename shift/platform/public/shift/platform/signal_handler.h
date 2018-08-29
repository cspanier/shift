#ifndef SHIFT_PLATFORM_SIGNALHANDLER_H
#define SHIFT_PLATFORM_SIGNALHANDLER_H

#include <string>
#include <shift/core/boost_disable_warnings.h>
#include <boost/signals2.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/core/singleton.h"

namespace shift::platform
{
enum class signal
{
  // interactive attention signal sent by pressing ctrl+c
  sigint,
  // Caused by abort, assert, or ctrl+break on Windows
  sigabrt,
  // Termination request
  sigterm
};

///
class signal_handler
: public core::singleton<signal_handler, core::create::on_stack>
{
public:
  /// Default constructor.
  signal_handler();

  /// Destructor.
  ~signal_handler();

  boost::signals2::signal<bool(signal signal)> handler;
};
}

#endif
