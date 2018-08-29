#define BOOST_USE_WINDOWS_H
#include "shift/platform/signal_handler.h"
#include <consoleapi.h>

namespace shift::platform
{
long __stdcall console_handler_routine(unsigned int ctrlType)
{
  auto& handler = signal_handler::singleton_instance();

  switch (ctrlType)
  {
  case CTRL_C_EVENT:
    return *handler.handler(signal::sigint);

  case CTRL_BREAK_EVENT:
    return *handler.handler(signal::sigabrt);

  case CTRL_CLOSE_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_SHUTDOWN_EVENT:
    return *handler.handler(signal::sigterm);
  }
  return false;
}

signal_handler::signal_handler()
{
  SetConsoleCtrlHandler((PHANDLER_ROUTINE)&console_handler_routine, 1);
}

signal_handler::~signal_handler()
{
  SetConsoleCtrlHandler((PHANDLER_ROUTINE)&console_handler_routine, 0);
}
}
