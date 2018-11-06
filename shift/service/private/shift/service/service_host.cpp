#define BOOST_BIND_NO_PLACEHOLDERS
#include "shift/service/service_host.hpp"
#include "shift/service/service_host_impl.hpp"
#include "shift/service/basic_service.hpp"
#include <shift/network/network_host.hpp>
#include <shift/log/log.hpp>
#include <shift/platform/environment.hpp>
#include <iomanip>

namespace shift::service
{
service_host::service_host() : _impl(std::make_unique<impl>(*this))
{
}

service_host::~service_host() = default;

void service_host::bind_addresses(
  std::vector<boost::asio::ip::address> addresses)
{
  _impl->bind_addresses = std::move(addresses);
}

void service_host::multicast_addresses(
  boost::asio::ip::address_v4 multicast_address4,
  boost::asio::ip::address_v6 multicast_address6)
{
  _impl->multicast_address4 = std::move(multicast_address4);
  _impl->multicast_address6 = std::move(multicast_address6);
}

bool service_host::start(serialization::protocol_version_t protocol_version,
                         serialization::protocol_version_t required_version)
{
  return _impl->start(protocol_version, required_version);
}

void service_host::stop()
{
  std::lock_guard lock(_impl->start_mutex);
  /// ToDo: Loop through all registered services and close them.

  for (auto& multicast_socket : _impl->multicast_sockets)
    multicast_socket->close();
  _impl->multicast_sockets.clear();

  _impl->running = false;
}

bool service_host::running() const
{
  return _impl->running;
}

void service_host::publish()
{
  _impl->publish();
}

const std::vector<boost::asio::ip::address>& service_host::bind_addresses()
  const
{
  return _impl->bind_addresses;
}

std::pair<const boost::asio::ip::address_v4&,
          const boost::asio::ip::address_v6&>
service_host::multicast_addresses() const
{
  return std::make_pair(std::ref(_impl->multicast_address4),
                        std::ref(_impl->multicast_address6));
}

serialization::protocol_version_t service_host::protocol_version() const
{
  return _impl->protocol_version;
}

guid_t service_host::guid() const
{
  return _impl->host_guid;
}

void service_host::guid(guid_t new_guid)
{
  _impl->host_guid = new_guid;
}

void service_host::register_service(basic_service& service)
{
  std::lock_guard service_client_lock(_impl->service_client_mutex);
  if (service.is_provider())
  {
    if (service_host::singleton_instance().debug_service_registration)
    {
      log::debug() << "Register service provider 0x" << std::hex << std::setw(8)
                   << std::setfill('0') << service.service_uid();
    }
    _impl->service_providers.insert(&service);
  }
  else
  {
    if (service_host::singleton_instance().debug_service_registration)
    {
      log::debug() << "Register service client 0x" << std::hex << std::setw(8)
                   << std::setfill('0') << service.service_uid();
    }
    _impl->service_clients.insert(&service);

    std::lock_guard remote_service_lock(_impl->known_remote_services_mutex);

    for (const auto& known_remote_service : _impl->known_remote_services)
    {
      if (known_remote_service.service_uid == service.service_uid())
      {
        service.add_service_provider(
          known_remote_service.host_guid, known_remote_service.service_guid,
          known_remote_service.interface_uids, known_remote_service.service_tag,
          known_remote_service.local_address,
          boost::asio::ip::tcp::endpoint(known_remote_service.remote_address,
                                         known_remote_service.remote_port));
      }

      // known_remote_service_iter->lastSeen =
      // std::chrono::system_clock::now();
    }
  }
}

void service_host::deregister_service(basic_service& service)
{
  std::lock_guard lock(_impl->service_client_mutex);
  if (service.is_provider())
  {
    if (service_host::singleton_instance().debug_service_registration)
    {
      log::debug() << "Deegister service provider 0x" << std::hex
                   << std::setw(8) << std::setfill('0')
                   << service.service_uid();
    }
    _impl->service_providers.erase(&service);
  }
  else
  {
    if (service_host::singleton_instance().debug_service_registration)
    {
      log::debug() << "Deegister service client 0x" << std::hex << std::setw(8)
                   << std::setfill('0') << service.service_uid();
    }
    _impl->service_clients.erase(&service);
  }
}
}
