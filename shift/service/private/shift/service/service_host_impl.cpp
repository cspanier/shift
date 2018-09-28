#define BOOST_BIND_NO_PLACEHOLDERS
#include "shift/service/service_host_impl.h"
#include "shift/service/basic_service.h"
#include "shift/service/detail/guid_generator.h"
#include <shift/network/network_host.h>
#include <shift/network/socket_base.h>
#include <shift/network/tcp_socket_client.h>
#include <shift/network/tcp_socket_listener.h>
#include <shift/network/udp_socket.h>
#include <shift/serialization/pair.h>
#include <shift/serialization/compact/inputarchive.h>
#include <shift/serialization/compact/outputarchive.h>
#include <shift/log/log.h>
#include <shift/platform/environment.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/iostreams/device/back_inserter.hpp>
#include <shift/core/boost_restore_warnings.h>

#include <iostream>

namespace shift::service
{
service_host::impl::impl(service_host& host)
: host(host), host_guid(detail::generate_guid())
{
}

service_host::impl::~impl()
{
  auto& network_host = network::network_host::singleton_instance();
  network_host.on_update_statistics = nullptr;
}

bool service_host::impl::start(
  serialization::protocol_version_t new_protocol_version,
  serialization::protocol_version_t new_required_version)
{
  std::lock_guard lock(start_mutex);

  if (running)
    return false;

  if (bind_addresses.empty())
  {
    bind_addresses.emplace_back(
      boost::asio::ip::address_v4::from_string("0.0.0.0"));
    bind_addresses.emplace_back(
      boost::asio::ip::address_v6::from_string("0::0"));
  }
  protocol_version = new_protocol_version;
  required_version = new_required_version;

  using namespace std::placeholders;
  for (const auto& bind_address : bind_addresses)
  {
    auto multicast_socket = std::make_shared<network::udp_socket>();
    multicast_socket->on_receive =
      std::bind(&service_host::impl::on_receive_multicast, this, _1, _2, _3);
    if ((bind_address.is_v4() &&
         multicast_socket->open(bind_address, multicast_port,
                                multicast_address4)) ||
        (bind_address.is_v6() &&
         multicast_socket->open(bind_address, multicast_port,
                                multicast_address6)))
    {
      multicast_sockets.emplace_back(std::move(multicast_socket));
    }
    else
    {
      for (auto& socket : multicast_sockets)
        socket->close();
      multicast_sockets.clear();
      return false;
    }
  }

  running = true;
  return true;
}

void service_host::impl::publish()
{
  std::lock_guard lock(service_provider_mutex);
  auto count = static_cast<std::uint16_t>(service_providers.size());
  BOOST_ASSERT(service_providers.size() <=
               std::numeric_limits<decltype(count)>::max());

  for (auto& multicast_socket : multicast_sockets)
  {
    auto bind_address = multicast_socket->local_endpoint().address();
    std::vector<char> buffer;
    {
      serialization::compact_output_archive<> archive(1);
      archive.push(boost::iostreams::back_inserter(buffer));
      archive << host_guid << count;
      for (const auto& provider : service_providers)
      {
        archive << provider->service_guid() << provider->service_uid()
                << provider->interface_uids() << provider->service_tag()
                << provider->service_port(bind_address);
      }
    }

    if (bind_address.is_v4())
    {
      multicast_socket->post(
        std::move(buffer),
        boost::asio::ip::udp::endpoint(multicast_address4, multicast_port));
    }
    else
    {
      multicast_socket->post(
        std::move(buffer),
        boost::asio::ip::udp::endpoint(multicast_address6, multicast_port));
    }
  }
}

void service_host::impl::on_receive_multicast(
  network::udp_socket& socket, boost::asio::ip::udp::endpoint sender,
  std::vector<char> buffer)
{
  remote_service_provider service_provider;
  std::uint16_t remote_service_count;
  serialization::compact_input_archive<> archive(1);
  archive.push(boost::iostreams::array_source(buffer.data(), buffer.size()));
  archive >> service_provider.host_guid >> remote_service_count;
  /// ToDo: Catch exceptions.
  //{
  //  if (service_host::singleton_instance().debug_multicasts)
  //    log::warning() << "Ignoring ill-formed multicast message.";
  //  return;
  //}

  if (service_provider.host_guid == host_guid)
    return;  // Drop own packets.

  // service_provider.lastSeen = std::chrono::system_clock::now();

  if (remote_service_count != 0u)
  {
    std::lock_guard service_client_lock(service_client_mutex);
    std::lock_guard remote_service_lock(known_remote_services_mutex);

    for (; remote_service_count > 0; --remote_service_count)
    {
      archive >> service_provider.service_guid >>
        service_provider.service_uid >> service_provider.interface_uids >>
        service_provider.service_tag >> service_provider.remote_port;
      service_provider.local_address = socket.local_endpoint().address();
      service_provider.remote_address = sender.address();

      // Make sure each remote service is only added once.
      if (known_remote_services.find(service_provider.service_guid) !=
          known_remote_services.end())
      {
        // known_remote_service_iter->lastSeen =
        // std::chrono::system_clock::now();
        continue;
      }
      known_remote_services.insert(service_provider);

      for (auto& service_client : service_clients)
      {
        if (service_provider.service_uid == service_client->service_uid())
        {
          service_client->add_service_provider(
            service_provider.host_guid, service_provider.service_guid,
            service_provider.interface_uids, service_provider.service_tag,
            service_provider.local_address,
            boost::asio::ip::tcp::endpoint(service_provider.remote_address,
                                           service_provider.remote_port));
        }
      }
    }
  }
}
}
