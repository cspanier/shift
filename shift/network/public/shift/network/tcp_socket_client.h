#ifndef SHIFT_NETWORK_TCPSOCKETCLIENT_H
#define SHIFT_NETWORK_TCPSOCKETCLIENT_H

#include <memory>
#include <shift/core/boost_disable_warnings.h>
#include <boost/asio/ip/tcp.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/network/network.h"
#include "shift/network/socket_base.h"
#include "shift/network/tcp_socket_listener.h"

namespace shift::network
{
class tcp_socket_client final
: public socket_base,
  public std::enable_shared_from_this<tcp_socket_client>,
  public boost::noncopyable
{
public:
  /// Construct the client connection.
  tcp_socket_client(network::options options);

  /// Destructor.
  ~tcp_socket_client();

  /// Returns the parent socket which created this one, or nullptr if this is
  /// a root socket.
  std::shared_ptr<tcp_socket_listener> parent() const;

  /// Closes the connection to a remote server.
  /// @remarks:
  ///   On system shutdown you need to wait until all queued coroutines are
  ///   finished.
  void close();

  /// Initiates a connection to the remote server.
  bool connect(const boost::asio::ip::address& remote_address,
               std::uint16_t remote_port);

  /// Pushes the argument to the queue of buffers that are to be written
  /// asynchronously to the data stream.
  void post(std::vector<char> buffer);

  /// Pushes a buffer to the internal queue. This method may be used to
  /// manually inject data as if it was received through this socket.
  void queue(std::vector<char> buffer);

  /// Returns the local endpoint which the socket was bound to upon
  /// connect/accept.
  boost::asio::ip::tcp::endpoint local_endpoint() const;

  /// Returns the remote endpoint which the socket is connected to.
  boost::asio::ip::tcp::endpoint remote_endpoint() const;

public:
  /// Event handler which gets called when a Tcp connect attempt failed.
  on_connect_failed_t on_connect_failed;

  /// Event handler which gets called once a Tcp connection is established.
  on_connected_t on_connected;

  /// Event handler which gets called when a Tcp connection gets closed or a
  /// connect attempt failed.
  on_closed_t on_closed;

  /// Event handler which gets called when a received message block is being
  /// processed.
  /// @see network_host::receive.
  on_receive_packet_t on_receive;

protected:
  class impl;
  friend class tcp_socket_listener::impl;

  /// Sets the parent socket.
  void parent(std::shared_ptr<tcp_socket_listener> other);

  /// Adopt the externally connected socket.
  void adopt();

  std::unique_ptr<impl> _impl;
};
}

#endif
