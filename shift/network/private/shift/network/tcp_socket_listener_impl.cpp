#include "shift/network/tcp_socket_listener_impl.hpp"
#include "shift/network/tcp_socket_client_impl.hpp"
#include "shift/network/tcp_socket_listener.hpp"
#include "shift/network/tcp_socket_client.hpp"
#include "shift/network/network_host.hpp"
#include <shift/log/log.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <functional>

namespace shift::network
{
tcp_socket_listener::impl::impl(network::options options)
: host(network_host::singleton_instance()),
  options(options),
  acceptor(host.io_context()),
  listen_strand(host.io_context())
{
  closing.clear();
}

tcp_socket_listener::impl::~impl() = default;

bool tcp_socket_listener::impl::listen(tcp_socket_listener& listener_socket,
                                       boost::asio::ip::address bind_address,
                                       std::uint16_t bind_port)
{
  boost::asio::ip::tcp::endpoint local_endpoint{bind_address, bind_port};
  boost::system::error_code error;
  acceptor.open(local_endpoint.protocol());
  acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor.bind(local_endpoint);
  acceptor.listen(boost::asio::socket_base::max_connections, error);

  if (error)
  {
    log::error() << "Failed opening Tcp listening socket on " << local_endpoint
                 << ".";
    return false;
  }

  boost::asio::spawn(
    listen_strand, boost::bind(&tcp_socket_listener::impl::listen_routine, this,
                               listener_socket.shared_from_this(), _1));
  return true;
}

void tcp_socket_listener::impl::close()
{
  if (closing.test_and_set())
    return;

  if (host.debug_socket_lifetime)
    network_log() << "Closing listening connection.";
  if (acceptor.is_open())
    acceptor.cancel();
}

void tcp_socket_listener::impl::listen_routine(
  std::shared_ptr<tcp_socket_listener> listener_socket,
  boost::asio::yield_context yield)
{
  boost::system::error_code error;

  for (;;)
  {
    // Create a new tcp_socket_client to handle the new connection.
    auto client_socket = std::make_shared<tcp_socket_client>(options);
    client_socket->parent(listener_socket);

    acceptor.async_accept(client_socket->_impl->underlying_socket,
                          yield[error]);
    switch (error.value())
    {
    case boost::system::errc::success:
      break;

    case boost::asio::error::operation_aborted:
      close();
      return;

    default:
      log::error() << "Failed accepting an incoming connection ("
                   << error.message() << ").";
      continue;
    }
    client_socket->adopt();
  }
}
}
