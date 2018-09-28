#define BOOST_BIND_NO_PLACEHOLDERS
#include "shift/service/detail/udp_service.h"
#include "shift/service/detail/guid_generator.h"
#include "shift/service/detail/udp_connection.h"
#include "shift/service/service_host.h"
#include "shift/service/basic_service.h"
#include "shift/service/message_envelope.h"
#include <shift/network/network_host.h>
#include <shift/serialization/message.h>
#include <shift/serialization/compact/inputarchive.h>
#include <shift/serialization/compact/outputarchive.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/iostreams/device/back_inserter.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <iomanip>

namespace shift::service::detail
{
thread_local udp_connection* udp_service::_caller = nullptr;

udp_service::udp_service(basic_service& service, bool stream)
: service_impl(service),
  _stream(stream),
  _timeoutTimer(network::network_host::singleton_instance().io_context())
{
}

udp_service::~udp_service()
{
  stop();
}

bool udp_service::start()
{
  using namespace std::placeholders;

  auto& service_host = service_host::singleton_instance();
  for (const auto& bind_address : service_host.bind_addresses())
  {
    if ((!bind_address.is_v4() && multicast_address.is_v4()) ||
        (bind_address.is_v4() && !multicast_address.is_v4()))
    {
      continue;
    }
    auto socket = std::make_shared<network::udp_socket>();
    socket->on_receive = std::bind(&udp_service::on_receive, this, _1, _2, _3);
    socket->open(bind_address, service_port, multicast_address);
    _sockets.emplace_back(std::move(socket));
  }

  // Call check_disconnects once to avoide code duplication of queuing the
  // _timeoutTimer.
  return true;
}

void udp_service::stop()
{
  _timeoutTimer.cancel();

  for (auto& socket : _sockets)
  {
    socket->on_receive = nullptr;
    socket->close();
  }
  _sockets.clear();

  std::lock_guard lock(_connection_mutex);
  _connections.clear();
}

std::pair<guid_t, guid_t> udp_service::sender() const
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

boost::asio::ip::address udp_service::sender_address() const
{
  auto* caller = _caller;
  if (caller == nullptr)
  {
    BOOST_THROW_EXCEPTION(core::invalid_operation() << core::context_info(
                            "Calling sender() is only valid from within "
                            "the context of a service method."));
  }
  return caller->endpoint.address();
}

std::uint16_t udp_service::sender_port() const
{
  auto* caller = _caller;
  if (caller == nullptr)
  {
    BOOST_THROW_EXCEPTION(core::invalid_operation() << core::context_info(
                            "Calling sender() is only valid from within "
                            "the context of a service method."));
  }
  return caller->endpoint.port();
}

void udp_service::broadcast(const serialization::message& message)
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

  decltype(_connections) connections;
  {
    std::lock_guard lock(_connection_mutex);
    connections = _connections;
  }
  auto count = connections.size();
  if (count == 0u)
  {
    log::warning() << "Attempt to broadcast message, but no remote "
                      "service is currently connected.";
    return;
  }
  for (auto& connection : connections)
  {
    if (--count != 0u)
      connection->socket->post(buffer, connection->endpoint);
    else
      connection->socket->post(std::move(buffer), connection->endpoint);
  }
}

void udp_service::reply(const serialization::message& message)
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
  caller->socket->post(std::move(buffer), caller->endpoint);
}

void udp_service::send_to(guid_t remote_service_guid,
                          const serialization::message& message)
{
  decltype(_connections) connections;
  {
    std::lock_guard lock(_connection_mutex);
    connections = _connections;
  }
  for (auto& connection : connections)
  {
    if (connection->service_guid == remote_service_guid)
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
      connection->socket->post(std::move(buffer), connection->endpoint);
      return;
    }
  }
  log::warning() << "Attempt to send message to the remote service "
                 << remote_service_guid << ", which does not exist.";
}

void udp_service::send_to(const boost::asio::ip::udp::endpoint& target_endpoint,
                          const serialization::message& message)
{
  for (auto& socket : _sockets)
  {
    /// ToDo: Find a better way to check, whether this socket is a route to
    /// target_endpoint. It is likely wrong when using multiple NICs.
    if (socket->local_endpoint().protocol() == target_endpoint.protocol())
    {
      std::vector<char> buffer;
      {
        /// ToDo: Which protocol_version to use?!?
        serialization::compact_output_archive<> archive(1);
        archive.push(boost::iostreams::back_inserter(buffer));
        auto message_uid = message.unique_id();
        envelope->begin_write(archive, message_uid);
        archive << message;
        envelope->end_write(archive, message_uid);
      }
      socket->post(std::move(buffer), target_endpoint);
    }
  }
}

void udp_service::send_to_except(guid_t remote_service_guid,
                                 const serialization::message& message)
{
  decltype(_connections) connections;
  {
    std::lock_guard lock(_connection_mutex);
    connections = _connections;
  }
  for (auto& connection : connections)
  {
    if (connection->service_guid != remote_service_guid)
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
      connection->socket->post(std::move(buffer), connection->endpoint);
    }
  }
}

void udp_service::on_receive(network::udp_socket& socket,
                             const boost::asio::ip::udp::endpoint& sender,
                             std::vector<char> buffer)
{
  decltype(_connections) connections;
  {
    std::lock_guard lock(_connection_mutex);
    connections = _connections;
  }
  udp_connection* connection = nullptr;
  for (auto& existing_connection : connections)
  {
    if (existing_connection->endpoint == sender)
    {
      connection = existing_connection.get();
      break;
    }
  }
  if (connection == nullptr)
  {
    // We received a datagram from an unknown remote endpoint. To do things
    // as similar to the Tcp based service implementation as possible, we
    // simulate a "connect" request from the new remote socket even though
    // Udp is connection-less.
    auto new_connection = std::make_shared<udp_connection>();

    // Set the host guid to zero and generate a new service guid because
    // we are not using handshakes.
    new_connection->host_guid = 0;
    new_connection->service_guid = generate_guid();
    new_connection->socket = socket.shared_from_this();
    new_connection->endpoint = sender;
    new_connection->protocol_version = 1;
    new_connection->service_tag = 0;

    {
      std::lock_guard lock(_connection_mutex);
      _connections.push_back(new_connection);
    }
    if (service.on_connected_remote_service)
    {
      service.on_connected_remote_service(
        service, new_connection->host_guid, new_connection->service_guid,
        boost::asio::ip::tcp::endpoint(sender.address(), sender.port()));
    }
    connection = new_connection.get();
  }

  connection->timeout =
    std::chrono::high_resolution_clock::now() + service.timeout();

  /// ToDo: Get the protocol version negotiated with the remote service.
  _caller = connection;
  read_messages(std::move(buffer), connection->read_buffer,
                connection->protocol_version, _stream);
  _caller = nullptr;
}

bool udp_service::add_service_provider(
  guid_t remote_host_guid, guid_t remote_service_guid,
  interface_uids_t remote_interface_uids, service_tag_t remote_service_tag,
  boost::asio::ip::address local_address,
  boost::asio::ip::tcp::endpoint remote_endpoint)
{
  if (remote_interface_uids.first != interface_uids.second ||
      remote_interface_uids.second != interface_uids.first)
  {
    if (service.on_found_incompatible_remote_service)
    {
      service.on_found_incompatible_remote_service(
        service, remote_host_guid, remote_service_guid, remote_interface_uids,
        remote_endpoint);
    }
    return false;
  }
  if (service.on_found_remote_service &&
      !service.on_found_remote_service(service, remote_host_guid,
                                       remote_service_guid, service_uid,
                                       remote_service_tag, remote_endpoint))
  {
    return false;
  }

  auto connection = std::make_shared<udp_connection>();
  connection->host_guid = remote_host_guid;
  connection->service_guid = remote_service_guid;
  for (auto& socket : _sockets)
  {
    if (socket->local_endpoint().address() == local_address)
      connection->socket = socket;
  }
  BOOST_ASSERT(connection->socket);
  if (!connection->socket)
    return false;

  connection->endpoint = boost::asio::ip::udp::endpoint(
    remote_endpoint.address(), remote_endpoint.port());
  connection->protocol_version = 1;
  connection->service_tag = remote_service_tag;
  {
    std::lock_guard lock(_connection_mutex);
    connection->timeout =
      std::chrono::high_resolution_clock::now() + service.timeout();
    _connections.push_back(connection);
  }
  if (service.on_connected_remote_service)
  {
    service.on_connected_remote_service(service, connection->host_guid,
                                        connection->service_guid,
                                        remote_endpoint);
  }
  return true;
}

void udp_service::check_disconnects(const boost::system::error_code& /*error*/)
{
  using namespace std::placeholders;

  std::vector<std::shared_ptr<udp_connection>> disconnects;
  {
    std::lock_guard lock(_connection_mutex);
    auto now = std::chrono::high_resolution_clock::now();
    _connections.erase(
      std::remove_if(_connections.begin(), _connections.end(),
                     [&](auto current) -> bool {
                       if (current->timeout <= now)
                       {
                         disconnects.emplace_back(std::move(current));
                         return true;
                       }
                       else
                         return false;
                     }),
      _connections.end());
  }
  for (const auto& disconnect : disconnects)
  {
    if (service.on_disconnected_remote_service)
    {
      service.on_disconnected_remote_service(service, disconnect->host_guid,
                                             disconnect->service_guid);
    }
  }

  // Queue new check.
  _timeoutTimer.expires_from_now(boost::posix_time::milliseconds(
    std::chrono::duration_cast<std::chrono::milliseconds>(_timeout).count()));
  _timeoutTimer.async_wait(
    std::bind(&udp_service::check_disconnects, this, _1));
}

std::size_t udp_service::connected_services_count()
{
  std::lock_guard lock(_connection_mutex);
  return _connections.size();
}

std::chrono::high_resolution_clock::duration udp_service::timeout() const
{
  return _timeout;
}

void udp_service::timeout(std::chrono::high_resolution_clock::duration duration)
{
  _timeoutTimer.cancel();
  _timeout = duration;
  if (duration.count() > 0)
    check_disconnects(boost::system::error_code());
}
}
