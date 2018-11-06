#ifndef SHIFT_NETWORK_TCPSOCKETLISTENER_HPP
#define SHIFT_NETWORK_TCPSOCKETLISTENER_HPP

#include <memory>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/network/network.hpp"
#include "shift/network/socket_base.hpp"

namespace shift::network
{
/// A server-side connection which allows listening on sockets and
/// duplicating itself on each incomming connection to handle communication.
class tcp_socket_listener final
: public socket_base,
  public std::enable_shared_from_this<tcp_socket_listener>
{
public:
  /// Constructor creating the server instance.
  tcp_socket_listener(network::options options);

  /// Destructor.
  ~tcp_socket_listener();

  /// Closes the connection and eventually cancels any pending accept.
  /// @remarks:
  ///   On system shutdown you need to wait until all queued coroutines are
  ///   finished.
  void close();

  /// Starts listening on the server socket.
  /// @param bind_port
  ///   If this parameter is set to zero, the port will be chosen
  ///   automatically by the OS. You may query the port once listen has been
  ///   called using socket::port.
  bool listen(boost::asio::ip::address bind_address, std::uint16_t bind_port);

  /// Returns the local endpoint which the socket is listening on.
  boost::asio::ip::tcp::endpoint local_endpoint() const;

public:
  /// Event handler which gets called once a Tcp connection is established.
  on_connected_t on_connected;

  /// Event handler which gets called when a Tcp connection gets closed or a
  /// connect attempt failed.
  on_closed_t on_closed;

  class impl;

protected:
  std::unique_ptr<impl> _impl;
};
}

#endif
