#ifndef SHIFT_SERVICE_DETAIL_TCPCONNECTION_H
#define SHIFT_SERVICE_DETAIL_TCPCONNECTION_H

#include <memory>
#include <vector>
#include <shift/network/network.h>
#include <shift/network/tcp_socket_client.h>
#include <shift/serialization/types.h>
#include "shift/service/types.h"

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
