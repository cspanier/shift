#define BOOST_BIND_NO_PLACEHOLDERS
#include "shift/service/detail/tcp_service_provider.hpp"
#include "shift/service/detail/tcp_connection.hpp"
#include "shift/service/basic_service.hpp"
#include <shift/network/network_host.hpp>
#include <iomanip>

namespace shift::service::detail
{
tcp_service_provider::tcp_service_provider(basic_service& service)
: tcp_service(service)
{
  using namespace std::placeholders;
}

tcp_service_provider::~tcp_service_provider() = default;

std::uint16_t tcp_service_provider::specific_service_port(
  boost::asio::ip::address bind_address) const
{
  for (auto& listener : _listeners)
  {
    auto endpoint = listener->local_endpoint();
    if (endpoint.address() == bind_address)
      return endpoint.port();
  }
  return service_port;
}

bool tcp_service_provider::start()
{
  using namespace std::placeholders;

  if (!tcp_service::start())
    return false;

  auto& service_host = service_host::singleton_instance();

  for (const auto& bind_address : service_host.bind_addresses())
  {
    auto listener = std::make_shared<network::tcp_socket_listener>(
      network::option::no_delay | network::option::read_packet32 |
      network::option::write_packet32);
    listener->on_connected =
      std::bind(&tcp_service_provider::on_connected, this, _1);
    listener->on_closed =
      std::bind(&tcp_service_provider::on_closed, this, _1, _2);
    if (!listener->listen(bind_address, service_port))
      return false;
    _listeners.emplace_back(std::move(listener));
  }
  return true;
}

void tcp_service_provider::stop()
{
  for (auto& listener : _listeners)
  {
    listener->close();
    listener->on_connected = nullptr;
    listener->on_closed = nullptr;
  }
  _listeners.clear();
  tcp_service::stop();
}

void tcp_service_provider::on_connected(network::tcp_socket_client& socket)
{
  using namespace std::placeholders;

  std::lock_guard lock(_connection_mutex);
  auto connection = std::make_unique<tcp_connection>();
  connection->socket = socket.shared_from_this();
  socket.user_data(connection.get());

  if (use_handshake)
  {
    socket.on_receive =
      std::bind(&tcp_service_provider::on_receive_handshake, this, _1, _2);
    _pending_connections.push_back(std::move(connection));
  }
  else
  {
    socket.on_receive =
      std::bind(&tcp_service_provider::on_receive, this, _1, _2);
    _established_connections.push_back(std::move(connection));
  }
}

void tcp_service_provider::on_receive_handshake(
  network::tcp_socket_client& socket, std::vector<char> buffer)
{
  if (!receive_handshake(socket, std::move(buffer), true))
    return;

  using namespace std::placeholders;
  socket.on_receive =
    std::bind(&tcp_service_provider::on_receive, this, _1, _2);
}
}
