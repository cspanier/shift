#include "shift/network/udp_socket_impl.hpp"
#include "shift/network/udp_socket.hpp"
#include "shift/network/network_host_impl.hpp"
#include "shift/network/network_host.hpp"
#include <shift/log/log.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <functional>

namespace shift::network
{
udp_socket::impl::impl()
: host(network_host::singleton_instance()),
  underlying_socket(host.io_context()),
  read_write_strand(host.io_context())
{
  closing.clear();
}

udp_socket::impl::~impl() = default;

bool udp_socket::impl::open(udp_socket& socket,
                            boost::asio::ip::address bind_address,
                            std::uint16_t bind_port,
                            boost::asio::ip::address multicast_address)
{
  using namespace boost::asio::ip;

  BOOST_ASSERT(!underlying_socket.is_open());
  if (underlying_socket.is_open())
    return false;

  BOOST_ASSERT(!multicast_address.is_multicast() ||
               (bind_address.is_v4() && multicast_address.is_v4()) ||
               (bind_address.is_v6() && multicast_address.is_v6()));
  if (multicast_address.is_multicast() &&
      (bind_address.is_v4() != multicast_address.is_v4() ||
       bind_address.is_v6() != multicast_address.is_v6()))
  {
    return false;
  }

  local_endpoint = udp::endpoint(bind_address, bind_port);
#if defined(SHIFT_PLATFORM_WINDOWS)
  auto listen_endpoint = local_endpoint;
#elif defined(SHIFT_PLATFORM_LINUX)
  // Binding the socket to bind_address does not receive multicast messages.
  // udp::endpoint listen_endpoint(bind_address, bind_port);

  // Binding the socket to multicast_address only receives multicast messages.
  // udp::endpoint listen_endpoint(multicast_address, bind_port);

  // Binding the socket to any address (0.0.0.0 or 0::0) is the only option
  // that seems to work for Linux.
  udp::endpoint listen_endpoint;
  if (bind_address.is_v4())
    listen_endpoint = udp::endpoint(udp::v4(), bind_port);
  else if (bind_address.is_v6())
    listen_endpoint = udp::endpoint(udp::v6(), bind_port);
#else
#error Specify bind address to use on this platform.
#endif
  underlying_socket.open(listen_endpoint.protocol());
  underlying_socket.set_option(udp::socket::reuse_address(true));
  underlying_socket.set_option(multicast::enable_loopback(true));

  boost::system::error_code error_code;
  if (bind_address.is_v4())
  {
    underlying_socket.set_option(
      multicast::outbound_interface(bind_address.to_v4()), error_code);
  }
  else if (bind_address.is_v6())
  {
    // There is no platform independent way to find an interface number.
    underlying_socket.set_option(multicast::outbound_interface(0), error_code);
  }
  if (error_code)
  {
    underlying_socket.close();
    return false;
  }

  underlying_socket.bind(listen_endpoint, error_code);
  if (error_code)
  {
    underlying_socket.close();
    return false;
  }
  local_endpoint.port(underlying_socket.local_endpoint().port());

  if (multicast_address.is_multicast())
  {
    if (bind_address.is_v4())
    {
      underlying_socket.set_option(
        multicast::join_group(multicast_address.to_v4(), bind_address.to_v4()));
    }
    else if (bind_address.is_v6())
    {
      // There is no platform independent way to find an interface number.
      underlying_socket.set_option(
        multicast::join_group(multicast_address.to_v6(), 0));
    }
  }

  // Enter read loop.
  boost::asio::spawn(
    read_write_strand,
    boost::bind(&impl::read_routine, this, socket.shared_from_this(), _1));

  return true;
}

void udp_socket::impl::close(udp_socket& socket)
{
  if (closing.test_and_set())
    return;

  if (host.debug_socket_lifetime)
    network_log() << "Closing socket.";
  boost::system::error_code error;
  if (underlying_socket.is_open())
  {
    // Initiate graceful socket closure.
    underlying_socket.shutdown(boost::asio::ip::udp::socket::shutdown_both,
                               error);
    underlying_socket.close(error);
  }

  host.queue_closed(socket.shared_from_this(), error);
}

void udp_socket::impl::post(std::shared_ptr<udp_socket> socket,
                            std::vector<char> buffer,
                            const boost::asio::ip::udp::endpoint destination)
{
  outgoing_buffer_queue.push(std::make_pair(destination, std::move(buffer)));

  if (!parallel_write_pending)
  {
    boost::asio::spawn(read_write_strand,
                       boost::bind(&impl::write_routine, this, socket, _1));
  }
}

void udp_socket::impl::read_routine(std::shared_ptr<udp_socket> socket,
                                    boost::asio::yield_context yield)
{
  boost::system::error_code error;
  std::size_t length;

  while (underlying_socket.is_open())
  {
    std::vector<char> incomming_buffer;
    incomming_buffer.resize(max_datagram_size);
    boost::asio::ip::udp::endpoint sender;

    length = underlying_socket.async_receive_from(
      boost::asio::buffer(incomming_buffer.data(), incomming_buffer.size()),
      sender, yield[error]);
    if (error)
    {
      switch (error.value())
      {
      case boost::asio::error::basic_errors::operation_aborted:
        // Skip printing this error message.
        break;

      case boost::asio::error::basic_errors::connection_refused:
        // Ignore this error for Udp sockets.
        continue;

      case boost::asio::error::basic_errors::connection_reset:
        // Ignore this error for Udp sockets.
        continue;

      default:
        if (host.debug_exceptions)
        {
          network_log() << "Failed reading data from socket: "
                        << error.message() << " (" << error.value() << ")";
        }
      }

      socket->close();
      return;
    }
    if (length == 0)
      continue;
    incomming_buffer.resize(length);

    host._impl->bytes_received += length;
    ++host._impl->packets_received;

    if (host.debug_message_payload || socket->debug_message_payload)
    {
      network_log log;
      log << "<" << sender.address() << ":" << sender.port() << std::hex
          << std::setfill('0') << std::setw(2);
      for (auto c : incomming_buffer)
        log << " " << static_cast<unsigned int>(static_cast<std::uint8_t>(c));
    }

    host.queue_datagram(socket, std::move(incomming_buffer), sender);
  }
}

void udp_socket::impl::write_routine(std::shared_ptr<udp_socket> socket,
                                     boost::asio::yield_context yield)
{
  boost::system::error_code error;
  std::size_t length;

  parallel_write_pending = true;
  while (!outgoing_buffer_queue.empty() && underlying_socket.is_open())
  {
    PacketSize outgoing_buffer_size;

    const auto& next_buffer = outgoing_buffer_queue.front();
    {
      outgoing_buffer_size = static_cast<PacketSize>(next_buffer.second.size());
      if (outgoing_buffer_size > max_datagram_size)
      {
        log::error() << "Failed writing very large buffer to network "
                        "socket.";
        outgoing_buffer_queue.pop();
        continue;
      }
    }

    if (host.debug_message_payload || socket->debug_message_payload)
    {
      network_log log;
      log << ">" << next_buffer.first << std::hex << std::setfill('0')
          << std::setw(2);
      for (auto c : next_buffer.second)
        log << " " << static_cast<unsigned int>(static_cast<std::uint8_t>(c));
    }

    length = underlying_socket.async_send_to(
      boost::asio::const_buffers_1(next_buffer.second.data(),
                                   outgoing_buffer_size),
      next_buffer.first, yield[error]);
    if (error)
    {
      switch (error.value())
      {
      case boost::asio::error::basic_errors::operation_aborted:
        // Skip printing this error message.
        break;

      case boost::asio::error::basic_errors::connection_reset:
        // Failed reading data from socket: Connection reset by peer.
        break;

      default:
        if (host.debug_exceptions)
        {
          network_log() << "Failed writing data to network socket: "
                        << error.message() << " (" << error.value() << ")";
        }
        break;
      }
      socket->close();
      return;
    }
    BOOST_ASSERT(length == outgoing_buffer_size);

    host._impl->bytes_sent += length;
    ++host._impl->packets_sent;
    outgoing_buffer_queue.pop();
  }
  parallel_write_pending = false;
}
}
