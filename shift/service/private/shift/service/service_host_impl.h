#ifndef SHIFT_SERVICE_SERVICEHOSTIMPL_H
#define SHIFT_SERVICE_SERVICEHOSTIMPL_H

#include <cstdint>
#include <chrono>
#include <vector>
#include <set>
#include <mutex>
#include <shift/core/boost_disable_warnings.h>
#include <boost/asio/ip/udp.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/singleton.h>
#include <shift/network/network.h>
#include <shift/serialization/types.h>
#include "shift/service/types.h"
#include "shift/service/basic_service.h"
#include "shift/service/service_host.h"

namespace shift::service
{
///
struct remote_service_provider
{
  guid_t host_guid;
  guid_t service_guid;
  service_uid_t service_uid;
  interface_uids_t interface_uids;
  std::uint16_t service_tag;
  boost::asio::ip::address local_address;
  boost::asio::ip::address remote_address;
  std::uint16_t remote_port;

  // std::chrono::system_clock::time_point lastSeen;
};

struct service_guid_tag
{
};

using remote_service_provider_set = boost::multi_index_container<
  remote_service_provider,
  boost::multi_index::indexed_by<boost::multi_index::ordered_unique<
    boost::multi_index::tag<service_guid_tag>,
    BOOST_MULTI_INDEX_MEMBER(remote_service_provider, guid_t, service_guid)>>>;

class service_host::impl
{
public:
  /// Constructor.
  impl(service_host& host);

  /// Destructor.
  ~impl();

  /// @see service_host::start.
  bool start(serialization::protocol_version_t new_protocol_version,
             serialization::protocol_version_t new_required_version);

  /// @see service_host::publish.
  void publish();

  /// Event handler for receiving multicast messages send by other service
  /// hosts.
  void on_receive_multicast(network::udp_socket& socket,
                            boost::asio::ip::udp::endpoint sender,
                            std::vector<char> buffer);

public:
  service_host& host;
  std::mutex start_mutex;
  bool running = false;

  std::vector<boost::asio::ip::address> bind_addresses;
  guid_t host_guid;
  std::uint16_t multicast_port = 0x700;
  boost::asio::ip::address_v4 multicast_address4 =
    boost::asio::ip::address_v4::from_string("239.255.255.1");
  boost::asio::ip::address_v6 multicast_address6 =
    boost::asio::ip::address_v6::from_string("FF05::1337:1");
  serialization::protocol_version_t protocol_version;
  serialization::protocol_version_t required_version;

  std::vector<std::shared_ptr<network::udp_socket>> multicast_sockets;

  std::mutex service_client_mutex;
  std::set<basic_service*> service_clients;
  std::mutex service_provider_mutex;
  std::set<basic_service*> service_providers;

  std::mutex known_remote_services_mutex;
  remote_service_provider_set known_remote_services;
};
}

#endif
