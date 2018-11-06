#ifndef NEWORK_NETWORKHOST_H
#define NEWORK_NETWORKHOST_H

#include <cstdint>
#include <chrono>
#include <memory>
#include <atomic>
#include <functional>
#include <string>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/udp.hpp>
#include <shift/core/boost_restore_warnings.hpp>
/// ToDo: Boost asio pulls Windows.h, which defines a lot of retarded symbols...
///       Check how to stop boost from doing that.
#ifdef GetClassName
#undef GetClassName
#endif
#ifdef GetObject
#undef GetObject
#endif
#ifdef GetCommandLine
#undef GetCommandLine
#endif
#include <shift/core/types.hpp>
#include <shift/core/singleton.hpp>
#include "shift/network/network.hpp"
#include "shift/network/socket_base.hpp"
#include "shift/network/tcp_socket_client.hpp"
#include "shift/network/tcp_socket_listener.hpp"
#include "shift/network/udp_socket.hpp"

namespace boost::asio
{
class io_context;
}

namespace shift::network
{
struct statistics_t
{
  size_t bytes_sent;
  size_t bytes_received;
  size_t bytes_sent_per_second;
  size_t bytes_received_per_second;
  size_t packets_sent;
  size_t packets_received;
  size_t packets_sent_per_second;
  size_t packets_received_per_second;
};

/// A network host offers connection handling and unified addressing using
/// IDs.
class network_host final
: public core::singleton<network_host, core::create::using_new>
{
public:
  /// Destructor.
  ~network_host();

  /// Returns the list of local class A, B, and C IPv4 and site-local IPv6
  /// interface descriptors.
  std::vector<boost::asio::ip::address> local_interfaces();

  /// Starts a number of network processing threads working on boost asio's
  /// IO service.
  void start(size_t num_threads);

  /// Returns true until the host is ordered to stop execution.
  bool running() const;

  /// Instructs and waits for all IO threads to stop.
  void stop();

  /// Processes all received buffers by calling on_receive_buffer for each
  /// pending buffer.
  std::size_t receive(std::chrono::system_clock::duration min_timeout,
                      std::chrono::system_clock::duration max_timeout =
                        std::chrono::milliseconds(0));

  /// Copies the current network statistics.
  void statistics(statistics_t& statistics) const;

  /// Returns a reference to the boost IO service.
  boost::asio::io_context& io_context();

  bool debug_exceptions = false;
  bool debug_socket_lifetime = false;
  bool debug_message_payload = false;

public:
  /// Event handler which gets called each time a Tcp client socket or Udp
  /// socket gets closed.
  std::function<void(std::shared_ptr<socket_base> socket,
                     boost::system::error_code error)>
    on_closed_socket;

  /// Event handler which gets called each time the sent/received bytes/
  /// packets statistics are updated.
  std::function<void()> on_update_statistics;

  /// Event hander which gets called from one of the network processing
  /// threads to signal that a new buffer has been pushed onto the internal
  /// queue. This means that receive() will successfully process at least one
  /// buffer.
  /// @remarks
  ///   This event handler is useful in situations where you don't have a loop
  ///   which constantly calls receive. For example you may use it in
  ///   combination with QT and emit a signal to the GUI processing thread,
  ///   which calls receive and thus avoids synchronization issues.
  std::function<void()> on_receive_buffer;

private:
  friend class core::singleton<network_host, core::create::using_new>;
  friend class tcp_socket_client;
  friend class tcp_socket_listener;
  friend class udp_socket;

  /// Default constructor.
  network_host();

  /// Pushes a socket connection failed event to the internal queue.
  void queue_connection_failed(std::shared_ptr<tcp_socket_client> socket,
                               boost::system::error_code error);

  /// Pushes a socket connected event to the internal queue.
  void queue_connected(std::shared_ptr<tcp_socket_client> socket);

  /// Pushes a Tcp socket closed event to the internal queue.
  void queue_closed(std::shared_ptr<tcp_socket_client> socket,
                    boost::system::error_code error);

  /// Pushes a Udp socket closed event to the internal queue.
  void queue_closed(std::shared_ptr<udp_socket> socket,
                    boost::system::error_code error);

  /// Pushes a received packet to the internal queue. This method may be used
  /// to manually inject data as if it was received through a network socket.
  void queue_packet(std::shared_ptr<tcp_socket_client> socket,
                    std::vector<char> buffer);

  /// Pushes a received datagram to the internal queue. This method may be
  /// used to manually inject data as if it was received through a network
  /// socket.
  void queue_datagram(std::shared_ptr<udp_socket> socket,
                      std::vector<char> buffer,
                      boost::asio::ip::udp::endpoint sender);

private:
  bool _started = false;
  std::atomic<bool> _quit = ATOMIC_VAR_INIT(false);
  size_t _num_threads = 1;

  class impl;
  std::unique_ptr<impl> _impl;
};
}

#endif
