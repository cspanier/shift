#define BOOST_BIND_NO_PLACEHOLDERS
#include "shift/service/detail/tcp_service.hpp"
#include "shift/service/detail/tcp_connection.hpp"
#include "shift/service/service_host.hpp"
#include "shift/service/basic_service.hpp"
#include "shift/service/message_envelope.hpp"
#include <shift/network/udp_socket.hpp>
#include <shift/network/tcp_socket_client.hpp>
#include <shift/serialization/pair.hpp>
#include <shift/serialization/compact/inputarchive.hpp>
#include <shift/serialization/compact/outputarchive.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <iomanip>

namespace shift::service::detail
{
thread_local tcp_connection* tcp_service::_caller = nullptr;

tcp_service::tcp_service(basic_service& service) : service_impl(service)
{
}

tcp_service::~tcp_service()
{
  stop();
}

bool tcp_service::start()
{
  return true;
}

void tcp_service::stop()
{
  std::lock_guard lock(_connection_mutex);
  for (auto& connection : _established_connections)
  {
    auto socket = connection->socket;
    if (socket)
    {
      socket->on_connected = nullptr;
      socket->on_closed = nullptr;
      socket->on_receive = nullptr;
      socket->user_data(nullptr);
      socket->close();
    }
  }
  _established_connections.clear();

  for (auto& connection : _pending_connections)
  {
    auto socket = connection->socket;
    if (socket)
    {
      socket->on_connected = nullptr;
      socket->on_closed = nullptr;
      socket->on_receive = nullptr;
      socket->user_data(nullptr);
      socket->close();
    }
  }
  _pending_connections.clear();
}

std::pair<guid_t, guid_t> tcp_service::sender() const
{
  auto* caller = _caller;
  if (caller == nullptr)
  {
    BOOST_THROW_EXCEPTION(core::invalid_operation() << core::context_info(
                            "Calling sender() is only valid from within "
                            "the context of a service method."));
  }
  return std::make_pair(caller->host_guid, caller->service_guid);
}

boost::asio::ip::address tcp_service::sender_address() const
{
  auto* caller = _caller;
  if (caller == nullptr)
  {
    BOOST_THROW_EXCEPTION(core::invalid_operation() << core::context_info(
                            "Calling sender() is only valid from within "
                            "the context of a service method."));
  }
  return caller->socket->remote_endpoint().address();
}

std::uint16_t tcp_service::sender_port() const
{
  auto* caller = _caller;
  if (caller == nullptr)
  {
    BOOST_THROW_EXCEPTION(core::invalid_operation() << core::context_info(
                            "Calling sender() is only valid from within "
                            "the context of a service method."));
  }
  return caller->socket->remote_endpoint().port();
}

void tcp_service::broadcast(const serialization::message& message)
{
  /// ToDo: Where to get the debug information from?
  /// ToDo: Which protocol version to use?
  std::vector<char> buffer;
  {
    serialization::protocol_version_t protocol_version = 1;
    serialization::compact_output_archive<> archive(protocol_version);
    archive.push(boost::iostreams::back_inserter(buffer));
    auto message_uid = message.unique_id();
    envelope->begin_write(archive, message_uid);
    archive << message;
    envelope->end_write(archive, message_uid);
  }

  std::lock_guard lock(_connection_mutex);
  auto count = _established_connections.size();
  if (count == 0u)
  {
    log::warning() << "Attempt to broadcast message, but no remote "
                      "service is currently connected.";
    return;
  }
  for (auto& connection : _established_connections)
  {
    if (--count != 0u)
      connection->socket->post(buffer);
    else
      connection->socket->post(std::move(buffer));
  }
}

void tcp_service::reply(const serialization::message& message)
{
  auto* caller = _caller;
  if (caller == nullptr)
  {
    BOOST_ASSERT(false);
    log::error() << "Calling reply() is only valid from within the "
                    "context of a service method.";
    return;
  }
  std::vector<char> buffer;
  {
    serialization::compact_output_archive<> archive(caller->protocol_version);
    archive.push(boost::iostreams::back_inserter(buffer));
    auto message_uid = message.unique_id();
    envelope->begin_write(archive, message_uid);
    archive << message;
    envelope->end_write(archive, message_uid);
  }
  caller->socket->post(std::move(buffer));
}

void tcp_service::send_to(guid_t remote_service_guid,
                          const serialization::message& message)
{
  std::lock_guard lock(_connection_mutex);
  for (auto& connection : _established_connections)
  {
    if (connection->socket && connection->service_guid == remote_service_guid)
    {
      std::vector<char> buffer;
      {
        serialization::compact_output_archive<> archive(
          connection->protocol_version);
        archive.push(boost::iostreams::back_inserter(buffer));
        auto message_uid = message.unique_id();
        envelope->begin_write(archive, message_uid);
        archive << message;
        envelope->end_write(archive, message_uid);
      }
      connection->socket->post(std::move(buffer));
      return;
    }
  }
  log::warning() << "Attempt to send message to the remote service "
                 << remote_service_guid << ", which does not exist.";
}

void tcp_service::send_to(
  const boost::asio::ip::udp::endpoint& /*target_endpoint*/,
  const serialization::message& /*message*/)
{
  /// ToDo: Throw illegal operation exception.
}

void tcp_service::send_to_except(guid_t remote_service_guid,
                                 const serialization::message& message)
{
  std::lock_guard lock(_connection_mutex);
  for (auto& connection : _established_connections)
  {
    if (connection->socket && connection->service_guid != remote_service_guid)
    {
      std::vector<char> buffer;
      {
        serialization::compact_output_archive<> archive(
          connection->protocol_version);
        archive.push(boost::iostreams::back_inserter(buffer));
        auto message_uid = message.unique_id();
        envelope->begin_write(archive, message_uid);
        archive << message;
        envelope->end_write(archive, message_uid);
      }
      connection->socket->post(std::move(buffer));
    }
  }
}

void tcp_service::on_receive(network::tcp_socket_client& socket,
                             std::vector<char> buffer)
{
  auto* connection = static_cast<tcp_connection*>(socket.user_data());
  if (connection == nullptr)
  {
    // Should never happen.
    BOOST_ASSERT(false);
    return;
  }

  /// ToDo: Get the protocol version negotiated with the remote service.
  _caller = connection;
  read_messages(std::move(buffer), connection->read_buffer,
                connection->protocol_version, true);
  _caller = nullptr;
}

void tcp_service::send_handshake(network::tcp_socket_client& socket)
{
  auto& host = service_host::singleton_instance();
  std::vector<char> buffer;
  {
    serialization::compact_output_archive<> archive(1);
    archive.push(boost::iostreams::back_inserter(buffer));
    archive << host.protocol_version() << host.guid() << service_guid
            << service_uid << interface_uids;
  }
  socket.post(std::move(buffer));
}

bool tcp_service::receive_handshake(network::tcp_socket_client& socket,
                                    std::vector<char> buffer,
                                    bool reply_handshake)
{
  serialization::compact_input_archive<> archive(1);
  archive.push(boost::iostreams::array_source(buffer.data(), buffer.size()));

  serialization::protocol_version_t protocol_version;
  guid_t remote_host_guid = 0;
  guid_t remote_service_guid = 0;
  service_uid_t remote_service_uid = 0;
  interface_uids_t remote_interface_uids = {0, 0};
  /// ToDo: Catch exceptions.
  archive >> protocol_version >> remote_host_guid >> remote_service_guid >>
    remote_service_uid >> remote_interface_uids;

  auto& host = service_host::singleton_instance();
  // if (protocol_version < required_version)
  //{
  //  /// ToDo: Print warning about incompatible client detected.
  //  return; // Remote client is incompatible.
  //}

  if (remote_service_uid != service.service_uid())
  {
    // The service types do not match.
    log::debug() << "Disconnecting remote service because its service "
                    "Uid does not match.";
    socket.close();
    return false;
  }

  if (remote_interface_uids.first != service.interface_uids().second ||
      remote_interface_uids.second != service.interface_uids().first)
  {
    // The service types do not match.
    log::debug() << "Disconnecting remote service because its interface "
                    "type does not match.";
    socket.close();
    return false;
  }

  if (service_host::singleton_instance().debug_handshakes)
  {
    log::debug() << "Service 0x" << std::hex << std::setw(8)
                 << std::setfill('0') << service.service_uid() << " accepted.";
  }
  auto* connection = static_cast<tcp_connection*>(socket.user_data());
  connection->host_guid = remote_host_guid;
  connection->service_guid = remote_service_guid;
  connection->protocol_version =
    std::min(host.protocol_version(), protocol_version);
  establish_connection(connection);
  if (reply_handshake)
    send_handshake(socket);
  if (service.on_connected_remote_service)
  {
    service.on_connected_remote_service(
      service, remote_host_guid, remote_service_guid, socket.remote_endpoint());
  }

  return true;
}

void tcp_service::establish_connection(tcp_connection* connection)
{
  std::lock_guard lock(_connection_mutex);
  for (auto connection_iter = _pending_connections.begin();
       connection_iter != _pending_connections.end(); ++connection_iter)
  {
    if (connection_iter->get() == connection)
    {
      _established_connections.push_back(std::move(*connection_iter));
      _pending_connections.erase(connection_iter);
      break;
    }
  }
}

void tcp_service::on_closed(network::socket_base& socket,
                            boost::system::error_code /*error*/)
{
  std::unique_ptr<tcp_connection> closing_connection;
  {
    std::lock_guard lock(_connection_mutex);
    for (auto connection_iter = _established_connections.begin();
         connection_iter != _established_connections.end(); ++connection_iter)
    {
      if (connection_iter->get() == socket.user_data())
      {
        closing_connection = std::move(*connection_iter);
        _established_connections.erase(connection_iter);
        break;
      }
    }
  }
  if (closing_connection)
  {
    socket.user_data(nullptr);

    if (service.on_disconnected_remote_service)
    {
      service.on_disconnected_remote_service(service,
                                             closing_connection->host_guid,
                                             closing_connection->service_guid);
    }
  }
}

std::size_t tcp_service::connected_services_count()
{
  std::lock_guard lock(_connection_mutex);
  return _established_connections.size();
}

std::chrono::high_resolution_clock::duration tcp_service::timeout() const
{
  /// ToDo: Throw illegal operation exception.
  return std::chrono::seconds(0);
}

void tcp_service::timeout(
  std::chrono::high_resolution_clock::duration /*duration*/)
{
  /// ToDo: Throw illegal operation exception.
}
}
