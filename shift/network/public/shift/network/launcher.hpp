#ifndef SHIFT_NETWORK_LAUNCHER_HPP
#define SHIFT_NETWORK_LAUNCHER_HPP

#include <chrono>
#include <thread>
#include <filesystem>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/program_options.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/platform/signal_handler.hpp>
#include <shift/core/exception.hpp>
#include "shift/network/network_host.hpp"

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

protected:
  void start() override
  {
    base_t::start();

    auto& host = network_host::singleton_create();
    host.debug_exceptions = _debug_exceptions;
    host.debug_socket_lifetime = _debug_socket_lifetime;
    host.debug_message_payload = _debug_message_payload;
    host.start(1);
  }

  void stop() noexcept override
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
