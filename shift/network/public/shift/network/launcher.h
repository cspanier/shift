#ifndef SHIFT_NETWORK_LAUNCHER_H
#define SHIFT_NETWORK_LAUNCHER_H

#include <chrono>
#include <thread>
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/platform/signal_handler.h>
#include <shift/core/exception.h>
#include "shift/network/network_host.h"

namespace shift::network
{
template <typename NextModule>
class launcher : public NextModule
{
public:
  using base_t = NextModule;

  launcher(int argc, char* argv[]) : base_t(argc, argv)
  {
    namespace opt = boost::program_options;

    base_t::_hidden_options.add_options()(
      "debug-network-exceptions",
      opt::value(&_debug_exceptions)->default_value(false),
      "Enable network exception debugging.");
    base_t::_hidden_options.add_options()(
      "debug-network-sockets",
      opt::value(&_debug_socket_lifetime)->default_value(false),
      "Enable network socket lifetime debugging.");
    base_t::_hidden_options.add_options()(
      "debug-network-payloads",
      opt::value(&_debug_message_payload)->default_value(false),
      "Enable network message payload debugging.");
  }

  ~launcher() override = default;

  void start() override
  {
    base_t::start();

    auto& host = network_host::singleton_create();
    host.debug_exceptions = _debug_exceptions;
    host.debug_socket_lifetime = _debug_socket_lifetime;
    host.debug_message_payload = _debug_message_payload;
    host.start(1);
  }

  void stop() override
  {
    if (network_host::singleton_instantiated())
    {
      network_host::singleton_instance().stop();
      network_host::singleton_destroy();
    }

    base_t::stop();
  }

private:
  bool _debug_exceptions = false;
  bool _debug_socket_lifetime = false;
  bool _debug_message_payload = false;
};
}

#endif
