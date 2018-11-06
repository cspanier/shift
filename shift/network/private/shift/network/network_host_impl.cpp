#include "shift/network/network_host_impl.hpp"
#include "shift/network/tcp_socket_client.hpp"
#include "shift/network/tcp_socket_listener.hpp"
#include "shift/network/udp_socket.hpp"
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/bind.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::network
{
network_host::impl::impl() = default;

void network_host::impl::statistics_update_handler(
  const boost::system::error_code& error)
{
  if (error)
    return;

  auto milliseconds =
    std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - last_statistics_update)
      .count();
  if (milliseconds <= 0)
    return;

  auto bytes_sent_now = bytes_sent.load();
  auto bytes_received_now = bytes_received.load();
  auto packets_sent_now = packets_sent.load();
  auto packets_received_now = packets_received.load();

  auto bytes_sent_last = bytes_sent_last_update.exchange(bytes_sent_now);
  auto bytes_received_last =
    bytes_received_last_update.exchange(bytes_received_now);
  auto packets_sent_last = packets_sent_last_update.exchange(packets_sent_now);
  auto packets_received_last =
    packets_received_last_update.exchange(packets_received_now);

  bytes_sent_per_second =
    (bytes_sent_now - bytes_sent_last) * 1000 / milliseconds;
  bytes_received_per_second =
    (bytes_received_now - bytes_received_last) * 1000 / milliseconds;
  packets_sent_per_second =
    (packets_sent_now - packets_sent_last) * 1000 / milliseconds;
  packets_received_per_second =
    (packets_received_now - packets_received_last) * 1000 / milliseconds;

  last_statistics_update = std::chrono::system_clock::now();
  statistics_update_timer->expires_from_now(boost::posix_time::seconds(1));
  statistics_update_timer->async_wait(
    boost::bind(&impl::statistics_update_handler, this, _1));

  auto& host = network_host::singleton_instance();
  if (host.on_update_statistics)
    host.on_update_statistics();
}

void network_host::impl::dispatch(socket_event& event)
{
  auto& host = network_host::singleton_instance();
  if (auto* received_packet = boost::get<received_packet_event>(&event);
      received_packet != nullptr)
  {
    BOOST_ASSERT(received_packet->socket);
    if (received_packet->socket && received_packet->socket->on_receive)
    {
      received_packet->socket->on_receive(*received_packet->socket,
                                          std::move(received_packet->buffer));
    }
    else
    {
      log::warning() << "Discarding network packet because there is no "
                        "on_receive handler set.";
    }
  }
  if (auto* receive_datagram = boost::get<received_datagram_event>(&event);
      receive_datagram != nullptr)
  {
    BOOST_ASSERT(receive_datagram->socket);
    if (receive_datagram->socket && receive_datagram->socket->on_receive)
    {
      receive_datagram->socket->on_receive(*receive_datagram->socket,
                                           receive_datagram->sender,
                                           std::move(receive_datagram->buffer));
    }
    else
    {
      log::warning() << "Discarding network packet because there is no "
                        "on_receive handler set.";
    }
  }
  else if (auto* connect_failed = boost::get<connect_failed_event>(&event);
           connect_failed != nullptr)
  {
    BOOST_ASSERT(connect_failed->socket);
    auto socket = connect_failed->socket;
    if (!socket)
      return;
    if (socket && socket->on_connect_failed)
      socket->on_connect_failed(*socket, connect_failed->error);
  }
  else if (auto* connected = boost::get<connected_event>(&event);
           connected != nullptr)
  {
    BOOST_ASSERT(connected->socket);
    auto socket = connected->socket;
    if (!socket)
      return;
    auto parent = socket->parent();
    if (parent && parent->on_connected)
      parent->on_connected(*socket);
    if (socket && socket->on_connected)
      socket->on_connected(*socket);
  }
  else if (auto* tcp_closed = boost::get<tcp_socket_closed_event>(&event);
           tcp_closed != nullptr)
  {
    BOOST_ASSERT(tcp_closed->socket);
    auto socket = tcp_closed->socket;
    if (!socket)
      return;
    auto parent = socket->parent();
    if (parent && parent->on_closed)
      parent->on_closed(*socket, tcp_closed->error);
    if (socket && socket->on_closed)
      socket->on_closed(*socket, tcp_closed->error);
    if (host.on_closed_socket)
      host.on_closed_socket(socket, tcp_closed->error);
  }
  else if (auto* udp_closed = boost::get<udp_socket_closed_event>(&event);
           udp_closed != nullptr)
  {
    BOOST_ASSERT(udp_closed->socket);
    auto socket = udp_closed->socket;
    if (!socket)
      return;
    if (socket && socket->on_closed)
      socket->on_closed(*socket, udp_closed->error);
    if (host.on_closed_socket)
      host.on_closed_socket(socket, udp_closed->error);
  }
}
}
