#ifndef SHIFT_NETWORK_TCPLISTENERSOCKETIMPL_H
#define SHIFT_NETWORK_TCPLISTENERSOCKETIMPL_H

#include <atomic>
#include <shift/core/boost_disable_warnings.h>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/network/network.h"
#include "shift/network/tcp_socket_listener.h"

namespace shift::network
{
/// A server-side connection which allows listening on sockets and
/// duplicating itself on each incomming connection to handle communication.
class tcp_socket_listener::impl
{
public:
  /// Constructor.
  impl(network::options options);

  /// Destructor.
  ~impl();

  /// Implementation of tcp_socket_listener::listen.
  bool listen(tcp_socket_listener& listener_socket,
              boost::asio::ip::address bind_address, std::uint16_t bind_port);

  /// Implementation of tcp_socket_listener::close.
  void close();

  /// The coroutine method responsible to listen on the server socket and
  /// forking new tcp_socket_listener instances for each incomming connection.
  void listen_routine(std::shared_ptr<tcp_socket_listener> listener_socket,
                      boost::asio::yield_context yield);

  network_host& host;
  network::options options;
  boost::asio::ip::tcp::acceptor acceptor;
  boost::asio::io_context::strand listen_strand;
  std::atomic_flag closing = ATOMIC_FLAG_INIT;
};
}

#endif
