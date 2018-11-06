#ifndef SHIFT_NETWORK_UDPSOCKETIMPL_HPP
#define SHIFT_NETWORK_UDPSOCKETIMPL_HPP

#include <queue>
#include <atomic>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/asio/spawn.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/network/udp_socket.hpp"

namespace shift::network
{
/// A Udp socket.
class udp_socket::impl
{
public:
  using PacketSize = std::uint16_t;

  /// Constructor.
  impl();

  /// Destructor.
  virtual ~impl();

  /// Implementation of udp_socket::open.
  bool open(udp_socket& socket, boost::asio::ip::address bind_address,
            std::uint16_t bind_port,
            boost::asio::ip::address multicast_address);

  /// Implementation of udp_socket::close.
  void close(udp_socket& socket);

  /// The actual implementation of udp_socket::post encapsulated using the
  /// read_write_strand thread synchronization object.
  void post(std::shared_ptr<udp_socket> socket, std::vector<char> buffer,
            boost::asio::ip::udp::endpoint destination);

  /// The coroutine method responsible to reading incomming data until the
  /// socket gets closed or until an error occurs.
  void read_routine(std::shared_ptr<udp_socket> socket,
                    boost::asio::yield_context yield);

  /// The coroutine method responsible to writing all queued outgoing data
  /// until the work is done, the socket gets closed or until an error
  /// occurs.
  void write_routine(std::shared_ptr<udp_socket> socket,
                     boost::asio::yield_context yield);

  network_host& host;
  boost::asio::ip::udp::socket underlying_socket;
  boost::asio::ip::udp::endpoint local_endpoint =
    boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0);

  boost::asio::io_context::strand read_write_strand;
  std::queue<std::pair<boost::asio::ip::udp::endpoint, std::vector<char>>>
    outgoing_buffer_queue;
  bool parallel_write_pending = false;
  std::atomic_flag closing = ATOMIC_FLAG_INIT;
};
}

#endif
