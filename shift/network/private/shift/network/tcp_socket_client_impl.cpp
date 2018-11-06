#include "shift/network/tcp_socket_client_impl.hpp"
#include "shift/network/network_host_impl.hpp"
#include "shift/network/network_host.hpp"
#include <shift/log/log.hpp>
#include <shift/core/hex_dump.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <functional>
#include <chrono>
#include <atomic>

namespace shift::network
{
tcp_socket_client::impl::impl(network::options options)
: host(network_host::singleton_instance()),
  options(options),
  underlying_socket(host.io_context()),
  resolver(host.io_context()),
  read_write_strand(host.io_context())
{
  closing.clear();
}

tcp_socket_client::impl::~impl() = default;

bool tcp_socket_client::impl::connect(tcp_socket_client& socket,
                                      boost::asio::ip::address remote_address,
                                      std::uint16_t remote_port)
{
  BOOST_ASSERT(!underlying_socket.is_open());
  if (underlying_socket.is_open())
    return false;

  boost::asio::spawn(
    host.io_context(),
    boost::bind(&tcp_socket_client::impl::connect_routine, this,
                socket.shared_from_this(),
                boost::asio::ip::tcp::endpoint{remote_address, remote_port},
                _1));
  return true;
}

void tcp_socket_client::impl::adopt(tcp_socket_client& socket)
{
  BOOST_ASSERT(underlying_socket.is_open());
  if (!underlying_socket.is_open())
    return;

  // Disable Nagle algorithm.
  underlying_socket.set_option(
    boost::asio::ip::tcp::no_delay(options & network::option::no_delay));

  host.queue_connected(socket.shared_from_this());

  // Enter the read loop.
  read(socket);
}

void tcp_socket_client::impl::close(tcp_socket_client& socket)
{
  if (closing.test_and_set())
    return;

  if (host.debug_socket_lifetime)
    network_log() << "Closing socket.";
  boost::system::error_code error;
  if (underlying_socket.is_open())
  {
    // Initiate graceful socket closure.
    underlying_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                               error);
    underlying_socket.close(error);
  }

  host.queue_closed(socket.shared_from_this(), error);
}

void tcp_socket_client::impl::read(tcp_socket_client& socket)
{
  if (options & network::option::read_packet_mask)
  {
    boost::asio::spawn(read_write_strand,
                       boost::bind(&tcp_socket_client::impl::read_packetRoutine,
                                   this, socket.shared_from_this(), _1));
  }
  else
  {
    boost::asio::spawn(read_write_strand,
                       boost::bind(&tcp_socket_client::impl::read_raw_routine,
                                   this, socket.shared_from_this(), _1));
  }
}

void tcp_socket_client::impl::post_routine(
  std::shared_ptr<tcp_socket_client> socket, std::vector<char> buffer)
{
  outgoing_buffer_queue.push(std::move(buffer));

  if (!parallel_write_pending)
  {
    if (options & network::option::write_packet_mask)
    {
      boost::asio::spawn(
        read_write_strand,
        boost::bind(&tcp_socket_client::impl::write_packetRoutine, this, socket,
                    _1));
    }
    else
    {
      boost::asio::spawn(
        read_write_strand,
        boost::bind(&tcp_socket_client::impl::write_raw_routine, this, socket,
                    _1));
    }
  }
}

void tcp_socket_client::impl::connect_routine(
  std::shared_ptr<tcp_socket_client> socket,
  boost::asio::ip::tcp::endpoint remote_endpoint,
  boost::asio::yield_context yield)
{
  boost::system::error_code error;

  // boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
  //  resolver.async_resolve(remote_endpoint, yield[error]);
  // if (error)
  //{
  //  log::error() << "Cannot resolve " << socket->remote_address() << ":"
  //                  << socket->remote_port() << " (" << error.message()
  //                  << ").";
  //  host.queue_connection_failed(socket, error);
  //  return;
  //}

  // endpoint_iterator = boost::asio::async_connect(
  //  underlying_socket.lowest_layer(), endpoint_iterator, yield[error]);
  underlying_socket.async_connect(remote_endpoint, yield[error]);
  if (error)
  {
    log::error() << "Failed connecting to " << remote_endpoint << " ("
                 << error.message() << ").";
    underlying_socket.close();
    host.queue_connection_failed(socket, error);
    return;
  }

  // Disable Nagle algorithm.
  underlying_socket.set_option(
    boost::asio::ip::tcp::no_delay(options & network::option::no_delay));

  host.queue_connected(socket);

  // Enter read loop.
  read(*socket);
}

void tcp_socket_client::impl::read_packetRoutine(
  std::shared_ptr<tcp_socket_client> socket, boost::asio::yield_context yield)
{
  boost::system::error_code error;
  std::size_t bytes_read;

  while (underlying_socket.is_open())
  {
    std::uint32_t incomming_buffer_size = 0;
    if (options & network::option::read_packet8)
    {
      std::uint8_t buffer_size = 0;
      bytes_read = boost::asio::async_read(
        underlying_socket,
        boost::asio::buffer(&buffer_size, sizeof(buffer_size)), yield[error]);
      incomming_buffer_size = buffer_size;
    }
    else if (options & network::option::read_packet16)
    {
      std::uint16_t buffer_size = 0;
      bytes_read = boost::asio::async_read(
        underlying_socket,
        boost::asio::buffer(&buffer_size, sizeof(buffer_size)), yield[error]);
      incomming_buffer_size = buffer_size;
    }
    else if (options & network::option::read_packet32)
    {
      std::uint32_t buffer_size = 0;
      bytes_read = boost::asio::async_read(
        underlying_socket,
        boost::asio::buffer(&buffer_size, sizeof(buffer_size)), yield[error]);
      incomming_buffer_size = buffer_size;
    }
    else
    {
      BOOST_ASSERT(false);
      return;
    }
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
          network_log() << "Failed reading data from socket: "
                        << error.message() << " (" << error.value() << ")";
        }
      }

      socket->close();
      return;
    }

    if (((options & network::option::read_packet8) &&
         (bytes_read != sizeof(std::uint8_t))) ||
        ((options & network::option::read_packet16) &&
         (bytes_read != sizeof(std::uint16_t))) ||
        ((options & network::option::read_packet32) &&
         (bytes_read != sizeof(std::uint32_t))))
    {
      continue;
    }

    /// ToDo: Add some size constraint here.
    std::vector<char> incomming_buffer;
    incomming_buffer.resize(incomming_buffer_size);

    auto buffer =
      boost::asio::buffer(incomming_buffer.data(), incomming_buffer_size);
    bytes_read =
      boost::asio::async_read(underlying_socket, buffer, yield[error]);
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
          network_log() << "Failed reading data from socket: "
                        << error.message() << " (" << error.value() << ")";
        }
      }

      socket->close();
      return;
    }
    BOOST_ASSERT(bytes_read == incomming_buffer_size);

    host._impl->bytes_received += bytes_read;
    ++host._impl->packets_received;

    if (host.debug_message_payload || socket->debug_message_payload)
    {
      network_log() << "<" << socket->remote_endpoint() << "\n"
                    << shift::core::hex_dump<16>(incomming_buffer);
    }

    // auto endpoint = underlying_socket.remote_endpoint();
    host.queue_packet(socket, std::move(incomming_buffer));
  }
}

void tcp_socket_client::impl::read_raw_routine(
  std::shared_ptr<tcp_socket_client> socket, boost::asio::yield_context yield)
{
  boost::system::error_code error;
  std::size_t bytes_read;

  while (underlying_socket.is_open())
  {
    /// ToDo: Add some size constraint here.
    std::vector<char> incomming_buffer;
    incomming_buffer.resize(4096, 0);

    auto buffer =
      boost::asio::buffer(incomming_buffer.data(), incomming_buffer.size());
    bytes_read = underlying_socket.async_read_some(buffer, yield[error]);
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
          network_log() << "Failed reading data from socket: "
                        << error.message() << " (" << error.value() << ")";
        }
      }

      socket->close();
      return;
    }
    incomming_buffer.resize(bytes_read);

    host._impl->bytes_received += bytes_read;
    ++host._impl->packets_received;

    if (host.debug_message_payload || socket->debug_message_payload)
    {
      network_log() << "<" << socket->remote_endpoint() << "\n"
                    << shift::core::hex_dump<16>(incomming_buffer);
    }

    // auto endpoint = underlying_socket.remote_endpoint();
    host.queue_packet(socket, std::move(incomming_buffer));
  }
}

void tcp_socket_client::impl::write_packetRoutine(
  std::shared_ptr<tcp_socket_client> socket, boost::asio::yield_context yield)
{
  boost::system::error_code error;
  std::size_t bytes_written;

  parallel_write_pending = true;
  while (!outgoing_buffer_queue.empty() && underlying_socket.is_open())
  {
    std::uint32_t outgoing_buffer_size = 0;
    if (options & network::option::write_packet8)
    {
      auto buffer_size =
        static_cast<std::uint8_t>(outgoing_buffer_queue.front().size());
      if (buffer_size < outgoing_buffer_queue.front().size())
      {
        log::error() << "Failed writing very large buffer to network "
                        "socket.";
        outgoing_buffer_queue.pop();
        continue;
      }
      bytes_written = boost::asio::async_write(
        underlying_socket,
        boost::asio::const_buffers_1(&buffer_size, sizeof(buffer_size)),
        yield[error]);
      BOOST_ASSERT(bytes_written == sizeof(buffer_size) || error);
      outgoing_buffer_size = buffer_size;
    }
    else if (options & network::option::write_packet16)
    {
      auto buffer_size =
        static_cast<std::uint16_t>(outgoing_buffer_queue.front().size());
      if (buffer_size < outgoing_buffer_queue.front().size())
      {
        log::error() << "Failed writing very large buffer to network "
                        "socket.";
        outgoing_buffer_queue.pop();
        continue;
      }
      bytes_written = boost::asio::async_write(
        underlying_socket,
        boost::asio::const_buffers_1(&buffer_size, sizeof(buffer_size)),
        yield[error]);
      BOOST_ASSERT(bytes_written == sizeof(buffer_size) || error);
      outgoing_buffer_size = buffer_size;
    }
    else if (options & network::option::write_packet32)
    {
      auto buffer_size =
        static_cast<std::uint32_t>(outgoing_buffer_queue.front().size());
      if (buffer_size < outgoing_buffer_queue.front().size())
      {
        log::error() << "Failed writing very large buffer to network "
                        "socket.";
        outgoing_buffer_queue.pop();
        continue;
      }
      bytes_written = boost::asio::async_write(
        underlying_socket,
        boost::asio::const_buffers_1(&buffer_size, sizeof(buffer_size)),
        yield[error]);
      BOOST_ASSERT(bytes_written == sizeof(buffer_size) || error);
      outgoing_buffer_size = buffer_size;
    }
    else
      BOOST_ASSERT(false);
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

    if (host.debug_message_payload || socket->debug_message_payload)
    {
      network_log() << ">" << socket->remote_endpoint() << "\n"
                    << shift::core::hex_dump<16>(outgoing_buffer_queue.front());
    }

    bytes_written = boost::asio::async_write(
      underlying_socket,
      boost::asio::const_buffers_1(outgoing_buffer_queue.front().data(),
                                   outgoing_buffer_size),
      yield[error]);
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
    BOOST_ASSERT(bytes_written == outgoing_buffer_size);

    host._impl->bytes_sent += bytes_written;
    ++host._impl->packets_sent;
    outgoing_buffer_queue.pop();
  }
  parallel_write_pending = false;
}

void tcp_socket_client::impl::write_raw_routine(
  std::shared_ptr<tcp_socket_client> socket, boost::asio::yield_context yield)
{
  boost::system::error_code error;
  std::size_t bytes_written;

  parallel_write_pending = true;
  while (!outgoing_buffer_queue.empty() && underlying_socket.is_open())
  {
    if (host.debug_message_payload || socket->debug_message_payload)
    {
      network_log() << ">" << socket->remote_endpoint() << "\n"
                    << shift::core::hex_dump<16>(outgoing_buffer_queue.front());
    }

    bytes_written = boost::asio::async_write(
      underlying_socket,
      boost::asio::const_buffers_1(outgoing_buffer_queue.front().data(),
                                   outgoing_buffer_queue.front().size()),
      yield[error]);
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
    BOOST_ASSERT(bytes_written == outgoing_buffer_queue.front().size());

    host._impl->bytes_sent += bytes_written;
    ++host._impl->packets_sent;
    outgoing_buffer_queue.pop();
  }

  parallel_write_pending = false;
}
}
