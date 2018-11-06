#ifndef SHIFT_SERVICE_DETAIL_UDPCONNECTION_HPP
#define SHIFT_SERVICE_DETAIL_UDPCONNECTION_HPP

#include <memory>
#include <vector>
#include <chrono>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/asio/ip/udp.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/network/network.hpp>
#include <shift/network/udp_socket.hpp>
#include <shift/serialization/types.hpp>
#include "shift/service/types.hpp"

namespace shift::service::detail
{
struct udp_connection
{
  guid_t host_guid = 0;
  guid_t service_guid = 0;
  std::shared_ptr<network::udp_socket> socket;
  boost::asio::ip::udp::endpoint endpoint;
  serialization::protocol_version_t protocol_version = 0;
  service_tag_t service_tag = 0;
  std::chrono::high_resolution_clock::time_point timeout;
  std::vector<char> read_buffer;
};
}

#endif
