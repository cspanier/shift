#ifndef SHIFT_SERVICE_BASICSERVICE_HPP
#define SHIFT_SERVICE_BASICSERVICE_HPP

#include <cstdint>
#include <memory>
#include <chrono>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <boost/asio/ip/udp.hpp>
#include <shift/serialization/types.hpp>
#include <shift/serialization/message.hpp>
#include <shift/network/network.hpp>
#include "shift/service/types.hpp"

namespace shift::service::detail
{
class service_impl;
class tcp_service;
class udp_service;
}

namespace shift::service
{
enum class service_protocol
{
  tcp,
  udp,
  udp_stream
};

class message_envelope;

/// Common base class for all services.
class basic_service
{
public:
  using on_found_remote_service_t = std::function<bool(
    basic_service& service, guid_t host_guid, guid_t service_guid,
    service_uid_t service_uid, service_tag_t service_tag,
    boost::asio::ip::tcp::endpoint endpoint)>;

  using on_found_incompatible_remote_service_t = std::function<void(
    basic_service& service, guid_t remote_host_guid, guid_t remote_service_guid,
    interface_uids_t remote_interface_uids,
    boost::asio::ip::tcp::endpoint endpoint)>;

  using on_connected_remote_service_t = std::function<void(
    basic_service& service, guid_t host_guid, guid_t service_guid,
    boost::asio::ip::tcp::endpoint endpoint)>;

  using on_disconnected_remote_service_t = std::function<void(
    basic_service& service, guid_t host_guid, guid_t service_guid)>;

  using on_receive_t = std::function<void(basic_service& service,
                                          serialization::message& message)>;

  class impl;

public:
  /// Constructor.
  basic_service(service_uid_t service_uid, interface_uids_t interface_uids,
                service_protocol protocol, bool is_provider,
                service_tag_t service_tag);

  /// Destructor.
  virtual ~basic_service();

  /// Returns the dynamic service GUID.
  guid_t service_guid() const;

  /// Returns the service's unique type id.
  service_uid_t service_uid() const;

  /// Returns the service interfaces' unique ids
  interface_uids_t interface_uids() const;

  /// Returns the network protocol used by this service.
  service_protocol protocol() const;

  /// Returns whether this service is a service provider.
  bool is_provider() const;

  /// Returns the service tag id.
  service_tag_t service_tag() const;

  /// Sets the local service port to use for either Tcp-based service
  /// providers or Udp-based services.
  /// @remarks
  ///   The setting defaults to 0, which indicates that the operating system
  ///   shall choose any free port available. The actually chosen port can be
  ///   requested using the service_port getter method once start has been
  ///   called.
  void service_port(std::uint16_t port);

  /// Returns the local port of the service's socket which is bound the the
  /// specified address.
  std::uint16_t service_port(boost::asio::ip::address bind_address) const;

  /// Sets the IP multicast address to join and use.
  /// @remarks
  ///   This only applys to Udp-based services.
  void multicast_address(boost::asio::ip::address address);

  /// Returns the IP multicast address used.
  boost::asio::ip::address multicast_address() const;

  /// Sets a timespan after which (Udp tunneled) connection-less remote hosts
  /// gets (virtually) disconnected.
  /// @param duration
  ///   A duration of zero seconds indicates that there won't be any timeout,
  ///   which is the default behaviour.
  /// @remarks
  ///   This call is only valid for UDP based services.
  void timeout(std::chrono::high_resolution_clock::duration duration);

  /// Returns the currently set timeout for (Udp tunneled) connection-less
  /// remote hosts.
  /// @returns
  ///   A duration of zero seconds indicates that there won't be any timeout,
  ///   which is the default behaviour.
  /// @remarks
  ///   This call is only valid for UDP based services.
  std::chrono::high_resolution_clock::duration timeout() const;

  /// Enables or disables broadcast messages containing information about this
  /// service.
  /// @remarks
  ///   These broadcast messages are sent using the service_host::publish
  ///   method.
  void broadcast_service(bool broadcast);

  /// Returns Enables or disables broadcast messages containing information
  /// about this service.
  bool broadcast_service() const;

  /// Enables or disables use of a built-in handshake protocol for new
  /// connections, which ensures compatibility of the two endpoints.
  void use_handshake(bool enable);

  /// Returns whether a built-in handshake protocol is to be used for new
  /// connections.
  bool use_handshake() const;

  /// Register a custom message envelope serializer to use.
  void envelope(std::unique_ptr<message_envelope> envelope);

  /// Returns the number of currently connected remote services.
  std::size_t connected_services_count() const;

  /// Starts searching for matching remote services.
  bool start();

  /// Stops searching for matching remote services.
  void stop();

  /// Returns the Guids of the remote host and remote service which called the
  /// last one of this service's routines.
  /// @remarks
  ///   This call is only valid from within one of the published
  ///   ServiceInterface routines.
  /// @throw
  ///   core::invalid_operation is thrown if this method is called from a
  ///   context other than described above.
  std::pair<guid_t, guid_t> sender() const;

  /// Returns the Guids of the remote host which called the last one of this
  /// service's routines.
  /// @remarks
  ///   This call is only valid from within one of the published
  ///   ServiceInterface routines.
  /// @throw
  ///   core::invalid_operation is thrown if this method is called from a
  ///   context other than described above.
  guid_t sender_host_guid() const;

  /// Returns the guid_t of the remote service which called the last one of
  /// this
  /// service's routines.
  /// @remarks
  ///   This call is only valid from within one of the published
  ///   ServiceInterface routines.
  /// @throw
  ///   core::invalid_operation is thrown if this method is called from a
  ///   context other than described above.
  guid_t sender_service_guid() const;

  /// Returns the IP address of the remote service which called the last one
  /// of this service's routines.
  /// @remarks
  ///   This call is only valid from within one of the published
  ///   ServiceInterface routines.
  /// @throw
  ///   core::invalid_operation is thrown if this method is called from a
  ///   context other than described above.
  boost::asio::ip::address sender_address() const;

  /// Returns the port of the remote service which called the last one of this
  /// service's routines.
  /// @remarks
  ///   This call is only valid from within one of the published
  ///   ServiceInterface routines.
  /// @throw
  ///   core::invalid_operation is thrown if this method is called from a
  ///   context other than described above.
  std::uint16_t sender_port() const;

  /// Broadcasts a messages to all connected services.
  void broadcast(const serialization::message& message);

  /// Sending a message to the service which called one of this service's
  /// routines.
  /// @remarks
  ///   This call is only valid from within one of the message event
  ///   handlers.
  void reply(const serialization::message& message);

  /// Sends a message to a specific remote service.
  void send_to(guid_t remote_service_guid,
               const serialization::message& message);

  /// Sends a message to a specific remote endpoint.
  /// @remarks
  ///   This call is only valid for UDP based services.
  void send_to(boost::asio::ip::udp::endpoint target_endpoint,
               const serialization::message& message);

  /// Sends a message to all connected remote services except the one with the
  /// specified guid.
  void send_to_except(guid_t remote_service_guid,
                      const serialization::message& message);

public:
  /// Event handler which gets called once a new remote service has been
  /// found. The handler may decide whether or not to accept the remote
  /// service by returning true or false, respectively.
  on_found_remote_service_t on_found_remote_service;

  ///
  on_found_incompatible_remote_service_t on_found_incompatible_remote_service;

  ///
  on_connected_remote_service_t on_connected_remote_service;

  ///
  on_disconnected_remote_service_t on_disconnected_remote_service;

  ///
  on_receive_t on_receive;

protected:
  friend class service_host;
  friend class detail::service_impl;

  /// This method gets called by the service::service_host each time a
  /// service provider with a matching service_uid is found.
  /// @remarks
  ///   The method filters out all services that have already been added,
  ///   then calls on_found_remote_service to ask the owner whether to accept
  ///   the remote service, and finally opens a new Tcp connection which
  ///   causes either on_connect_failed or on_connected to be called later on.
  bool add_service_provider(guid_t remote_host_guid, guid_t remote_service_guid,
                            interface_uids_t remote_interface_uids,
                            service_tag_t remote_service_tag,
                            boost::asio::ip::address local_address,
                            boost::asio::ip::tcp::endpoint remote_endpoint);

  ///
  virtual void read_message(
    serialization::compact_input_archive<boost::endian::order::big>& archive,
    serialization::message_uid_t uid) = 0;

  ///
  virtual void read_message(
    serialization::compact_input_archive<boost::endian::order::little>& archive,
    serialization::message_uid_t uid) = 0;

protected:
  service_protocol _protocol;
  std::unique_ptr<detail::service_impl> _impl;
};
}

#endif
