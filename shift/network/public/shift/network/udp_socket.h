#ifndef SHIFT_NETWORK_UDPSOCKET_H
#define SHIFT_NETWORK_UDPSOCKET_H

#include <memory>
#include <shift/core/boost_disable_warnings.h>
#include <boost/asio/ip/udp.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/network/network.h"
#include "shift/network/socket_base.h"

namespace shift::network
{
/// A Udp socket.
class udp_socket final : public socket_base,
                         public std::enable_shared_from_this<udp_socket>
{
public:
  static constexpr std::size_t max_datagram_size = 4096;

  /// Constructor.
  udp_socket();

  /// Destructor.
  ~udp_socket();

  /// Opens the Udp socket.
  /// @param multicast_address
  ///   If you specify a multicast address the socket attempts to join the
  ///   multicast group.
  bool open(const boost::asio::ip::address& bind_address,
            std::uint16_t bind_port,
            const boost::asio::ip::address& multicast_address =
              boost::asio::ip::address());

  /// Closes the socket.
  /// @remarks:
  ///   On system shutdown you need to wait until all queued coroutines are
  ///   finished.
  void close();

  /// Pushes the argument to the queue of buffers that are to be written
  /// asynchronously to the datagram socket.
  void post(std::vector<char> buffer,
            const boost::asio::ip::udp::endpoint& destination);

  /// Pushes a buffer to the internal queue. This method may be used to
  /// manually inject data as if it was received through this socket.
  void queue(std::vector<char> buffer,
             const boost::asio::ip::udp::endpoint& sender);

  /// Returns the local endpoint which the socket was bound to upon opening.
  boost::asio::ip::udp::endpoint local_endpoint() const;

public:
  /// Event handler which gets called when a received message block is being
  /// processed.
  /// @see network_host::receive.
  on_receive_datagram_t on_receive;

  /// Event handler which gets called when the socket gets closed.
  on_closed_t on_closed;

protected:
  class impl;
  std::unique_ptr<impl> _impl;
};
}

#endif
