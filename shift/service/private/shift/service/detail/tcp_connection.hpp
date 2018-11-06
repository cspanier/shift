#ifndef SHIFT_SERVICE_DETAIL_TCPCONNECTION_HPP
#define SHIFT_SERVICE_DETAIL_TCPCONNECTION_HPP

#include <memory>
#include <vector>
#include <shift/network/network.hpp>
#include <shift/network/tcp_socket_client.hpp>
#include <shift/serialization/types.hpp>
#include "shift/service/types.hpp"

namespace shift::service::detail
{
struct tcp_connection
{
  guid_t host_guid = 0;
  guid_t service_guid = 0;
  std::shared_ptr<network::tcp_socket_client> socket;
  serialization::protocol_version_t protocol_version = 0;
  service_tag_t service_tag = 0;
  std::vector<char> read_buffer;
};
}

#endif
