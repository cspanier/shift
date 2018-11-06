#define BOOST_BIND_NO_PLACEHOLDERS
#include "shift/service/detail/tcp_service_client.hpp"
#include "shift/service/detail/tcp_connection.hpp"
#include "shift/service/basic_service.hpp"
#include <shift/network/network_host.hpp>
#include <iomanip>

namespace shift::service::detail
{
tcp_service_client::tcp_service_client(basic_service& service)
: tcp_service(service)
{
}

tcp_service_client::~tcp_service_client() = default;

bool tcp_service_client::add_service_provider(
  guid_t remote_host_guid, guid_t remote_service_guid,
  interface_uids_t remote_interface_uids, service_tag_t remote_service_tag,
  boost::asio::ip::address /*local_address*/,
  boost::asio::ip::tcp::endpoint remote_endpoint)
{
  if (remote_interface_uids.first != interface_uids.second ||
      remote_interface_uids.second != interface_uids.first)
  {
    if (service.on_found_incompatible_remote_service)
    {
      service.on_found_incompatible_remote_service(
        service, remote_host_guid, remote_service_guid, remote_interface_uids,
        remote_endpoint);
    }
    return false;
  }
  if (service.on_found_remote_service &&
      !service.on_found_remote_service(service, remote_host_guid,
                                       remote_service_guid, service_uid,
                                       remote_service_tag, remote_endpoint))
  {
    return false;
  }

  std::lock_guard lock(_connection_mutex);
  auto connection = std::make_unique<tcp_connection>();
  connection->host_guid = remote_host_guid;
  connection->service_guid = remote_service_guid;

  connection->socket = std::make_shared<network::tcp_socket_client>(
    network::option::read_packet32 | network::option::write_packet32 |
    network::option::no_delay);
  connection->socket->user_data(connection.get());

  using namespace std::placeholders;
  connection->socket->on_connect_failed =
    std::bind(&tcp_service_client::on_connect_failed, this, _1, _2);
  connection->socket->on_connected =
    std::bind(&tcp_service_client::on_connected, this, _1);
  connection->socket->on_closed =
    std::bind(&tcp_service_client::on_closed, this, _1, _2);
  if (use_handshake)
  {
    connection->socket->on_receive =
      std::bind(&tcp_service_client::on_receive_handshake, this, _1, _2);
    connection->socket->connect(remote_endpoint.address(),
                                remote_endpoint.port());
    _pending_connections.push_back(std::move(connection));
  }
  else
  {
    connection->socket->on_receive =
      std::bind(&tcp_service_client::on_receive, this, _1, _2);
    connection->socket->connect(remote_endpoint.address(),
                                remote_endpoint.port());
    _established_connections.push_back(std::move(connection));
  }

  return true;
}

void tcp_service_client::on_connect_failed(network::tcp_socket_client& socket,
                                           boost::system::error_code /*error*/)
{
  // This may happen if the remote service died right after broadcasting
  // its address.
  /// ToDo: Print out some log message.
  std::lock_guard lock(_connection_mutex);
  for (auto connection_iter = _pending_connections.begin();
       connection_iter != _pending_connections.end(); ++connection_iter)
  {
    auto& connection = *connection_iter;
    if (connection.get() == socket.user_data())
    {
      socket.user_data(nullptr);
      _pending_connections.erase(connection_iter);
      break;
    }
  }
}

void tcp_service_client::on_connected(network::tcp_socket_client& socket)
{
  if (use_handshake)
    send_handshake(socket);
  else
    establish_connection(static_cast<tcp_connection*>(socket.user_data()));
}

void tcp_service_client::on_receive_handshake(
  network::tcp_socket_client& socket, std::vector<char> buffer)
{
  /// ToDo: What if the handshake and the first message are sent in one
  /// ethernet frame?
  if (!receive_handshake(socket, std::move(buffer), false))
    return;

  using namespace std::placeholders;
  socket.on_receive = std::bind(&tcp_service_client::on_receive, this, _1, _2);
}
}
