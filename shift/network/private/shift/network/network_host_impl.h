#ifndef SHIFT_NETWORK_NETWORKHOSTIMPL_H
#define SHIFT_NETWORK_NETWORKHOSTIMPL_H

#include <cstdint>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <shift/core/boost_disable_warnings.h>
#include <boost/variant.hpp>
#include <boost/asio.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/network/network.h"
#include "shift/network/network_host.h"

namespace shift::network
{
struct connected_event
{
  std::shared_ptr<tcp_socket_client> socket;
};

struct tcp_socket_closed_event
{
  std::shared_ptr<tcp_socket_client> socket;
  boost::system::error_code error;
};

struct udp_socket_closed_event
{
  std::shared_ptr<udp_socket> socket;
  boost::system::error_code error;
};

struct connect_failed_event
{
  std::shared_ptr<tcp_socket_client> socket;
  boost::system::error_code error;
};

struct received_packet_event
{
  std::shared_ptr<tcp_socket_client> socket;
  std::vector<char> buffer;
};

struct received_datagram_event
{
  std::shared_ptr<udp_socket> socket;
  boost::asio::ip::udp::endpoint sender;
  std::vector<char> buffer;
};

using socket_event =
  boost::variant<connected_event, tcp_socket_closed_event,
                 udp_socket_closed_event, connect_failed_event,
                 received_packet_event, received_datagram_event>;

class network_host::impl
{
public:
  /// Default constructor.
  impl();

  /// Updates each socket's *_per_second member fields.
  void statistics_update_handler(const boost::system::error_code& error);

  /// Dispatches a received event to its designated event handler.
  void dispatch(socket_event& event);

  std::unique_ptr<boost::asio::io_context> io_context;

  std::unique_ptr<boost::asio::deadline_timer> statistics_update_timer;
  std::chrono::system_clock::time_point last_statistics_update;
  std::atomic<size_t> bytes_sent = ATOMIC_VAR_INIT(0);
  std::atomic<size_t> bytes_received = ATOMIC_VAR_INIT(0);
  std::atomic<size_t> packets_sent = ATOMIC_VAR_INIT(0);
  std::atomic<size_t> packets_received = ATOMIC_VAR_INIT(0);
  std::atomic<size_t> bytes_sent_last_update = ATOMIC_VAR_INIT(0);
  std::atomic<size_t> bytes_received_last_update = ATOMIC_VAR_INIT(0);
  std::atomic<size_t> packets_sent_last_update = ATOMIC_VAR_INIT(0);
  std::atomic<size_t> packets_received_last_update = ATOMIC_VAR_INIT(0);
  std::size_t bytes_sent_per_second = 0;
  std::size_t bytes_received_per_second = 0;
  std::size_t packets_sent_per_second = 0;
  std::size_t packets_received_per_second = 0;

  std::vector<std::unique_ptr<std::thread>> _worker_threads;

  std::mutex _incomming_event_queue_mutex;
  std::condition_variable _incomming_event_signal;
  std::queue<std::pair<std::chrono::system_clock::time_point, socket_event>>
    _incomming_event_queue;
};
}

#endif
