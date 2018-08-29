#ifndef SHIFT_NETWORK_TCPCLIENTSOCKETIMPL_H
#define SHIFT_NETWORK_TCPCLIENTSOCKETIMPL_H

#include <queue>
#include <atomic>
#include <shift/core/boost_disable_warnings.h>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/network/tcp_socket_client.h"

namespace shift::network
{
class tcp_socket_client::impl
{
public:
  using PacketSize = std::uint32_t;

  /// Constructor.
  impl(network::options options);

  /// Destructor.
  ~impl();

  /// Implementation of tcp_socket_client::connect.
  bool connect(tcp_socket_client& socket,
               boost::asio::ip::address remote_address,
               std::uint16_t remote_port);

  /// Implementation of tcp_socket_client::adopt.
  void adopt(tcp_socket_client& socket);

  /// Implementation of tcp_socket_client::close.
  void close(tcp_socket_client& socket);

  /// Enters the read loop.
  void read(tcp_socket_client& socket);

  /// The actual implementation of tcp_socket_client::post encapsulated using
  /// the read_write_strand thread synchronization object.
  void post_routine(std::shared_ptr<tcp_socket_client> socket,
                    std::vector<char> buffer);

  /// The coroutine method which initiates a connection to the remote
  /// server.
  void connect_routine(std::shared_ptr<tcp_socket_client> socket,
                       boost::asio::ip::tcp::endpoint remote_endpoint,
                       boost::asio::yield_context yield);

  /// The coroutine method responsible to reading incomming data until the
  /// socket gets closed or until an error occurs.
  void read_packetRoutine(std::shared_ptr<tcp_socket_client> socket,
                          boost::asio::yield_context yield);

  /// The coroutine method responsible to reading incomming data until the
  /// socket gets closed or until an error occurs.
  void read_raw_routine(std::shared_ptr<tcp_socket_client> socket,
                        boost::asio::yield_context yield);

  /// The coroutine method responsible to writing all queued outgoing data
  /// until the work is done, the socket gets closed or until an error
  /// occurs.
  void write_packetRoutine(std::shared_ptr<tcp_socket_client> socket,
                           boost::asio::yield_context yield);

  /// The coroutine method responsible to writing all queued outgoing data
  /// until the work is done, the socket gets closed or until an error
  /// occurs.
  void write_raw_routine(std::shared_ptr<tcp_socket_client> socket,
                         boost::asio::yield_context yield);

  network_host& host;
  network::options options;
  boost::asio::ip::tcp::socket underlying_socket;
  std::shared_ptr<tcp_socket_listener> parent;
  boost::asio::ip::tcp::resolver resolver;
  boost::asio::io_context::strand read_write_strand;
  std::queue<std::vector<char>> outgoing_buffer_queue;

  bool parallel_write_pending = false;
  std::atomic_flag closing = ATOMIC_FLAG_INIT;
};
}

#endif
