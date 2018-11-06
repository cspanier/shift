#ifndef SHIFT_SERVICE_LAUNCHER_HPP
#define SHIFT_SERVICE_LAUNCHER_HPP

#include <memory>
#include <string>
#include <thread>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/program_options.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/service/types.hpp>
#include <shift/core/exception.hpp>
#include "shift/service/service_host.hpp"

namespace shift::service
{
template <typename NextModule>
class launcher : public NextModule
{
public:
  using base_t = NextModule;

  launcher(int argc, char* argv[]) : base_t(argc, argv)
  {
    namespace opt = boost::program_options;

    base_t::_visible_options.add_options()(
      "bind-address", opt::value(&_bind_addresses)->composing(),
      "IP addresses of the interfaces to bind to.");
    base_t::_visible_options.add_options()(
      "service-address-4",
      opt::value(&_service_address4)->default_value("239.255.255.5"),
      "IPv4 multicast address to use for service detection.");
    base_t::_visible_options.add_options()(
      "service-address-6",
      opt::value(&_service_address6)->default_value("FF05::74:A1E5:1"),
      "IPv6 multicast address to use for service detection.");
    base_t::_visible_options.add_options()(
      "service-host-guid", opt::value(&_service_host_guid)->default_value(0),
      "A global unique ID used to identify each application.");

    base_t::_hidden_options.add_options()(
      "debug-service-handshakes",
      opt::value(&_debug_handshakes)->default_value(false),
      "Enable service handshake debugging.");
    base_t::_hidden_options.add_options()(
      "debug-service-multicasts",
      opt::value(&_debug_multicasts)->default_value(false),
      "Enable service multicast debugging.");
    base_t::_hidden_options.add_options()(
      "debug-service-registration",
      opt::value(&_debug_service_registration)->default_value(false),
      "Enable service registration debugging.");
    base_t::_hidden_options.add_options()(
      "debug-service-calls",
      opt::value(&_debug_service_calls)->default_value(false),
      "Enable service call debugging.");

    base_t::_hidden_options.add_options()(
      "autostart-service-host",
      opt::value(&_autostart_service_host)->default_value(true),
      "Automatically start service host.");
  }

  ~launcher() override = default;

protected:
  void start() override
  {
    base_t::start();

    const serialization::protocol_version_t protocol_version = 1;
    const serialization::protocol_version_t required_version = 1;

    auto& host = service_host::singleton_create();
    host.debug_handshakes = _debug_handshakes;
    host.debug_multicasts = _debug_multicasts;
    host.debug_service_registration = _debug_service_registration;
    host.debug_service_calls = _debug_service_calls;
    {
      std::vector<boost::asio::ip::address> bind_addresses;
      for (auto& bind_address : _bind_addresses)
      {
        bind_addresses.push_back(
          boost::asio::ip::address::from_string(bind_address));
      }
      host.bind_addresses(std::move(bind_addresses));
    }
    host.multicast_addresses(
      boost::asio::ip::address_v4::from_string(_service_address4),
      boost::asio::ip::address_v6::from_string(_service_address6));
    if (_service_host_guid != 0)
      host.guid(_service_host_guid);
    if (_autostart_service_host)
    {
      while (!host.start(protocol_version, required_version))
      {
        log::info() << "Waiting for network interface to show up...";
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
  }

  void stop() noexcept override
  {
    if (service_host::singleton_instantiated())
    {
      service_host::singleton_instance().stop();
      service_host::singleton_destroy();
    }

    base_t::stop();
  }

private:
  std::vector<std::string> _bind_addresses;
  std::string _service_address4;
  std::string _service_address6;
  guid_t _service_host_guid;
  bool _debug_handshakes = false;
  bool _debug_multicasts = false;
  bool _debug_service_registration = false;
  bool _debug_service_calls = false;
  bool _autostart_service_host;
};
}

#endif
