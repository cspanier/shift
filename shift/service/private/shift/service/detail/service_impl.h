#ifndef SHIFT_SERVICE_DETAIL_SERVICEIMPL_H
#define SHIFT_SERVICE_DETAIL_SERVICEIMPL_H

#include <cstdint>
#include <memory>
#include <chrono>
#include <shift/core/boost_disable_warnings.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/serialization/types.h>
#include <shift/serialization/message.h>
#include <shift/network/network.h>
#include "shift/service/types.h"

namespace shift::service
{
class service;
}

namespace shift::service::detail
{
/// Common base class for all services.
class service_impl
{
public:
  /// Constructor.
  service_impl(basic_service& service);

  /// Destructor.
  virtual ~service_impl();

  /// @see service::service_port.
  virtual std::uint16_t specific_service_port(
    boost::asio::ip::address bind_address) const;

  /// Starts the service.
  virtual bool start() = 0;

  /// Stops the service.
  virtual void stop() = 0;

  /// This method gets called by the service::service_host each time a
  /// service provider with a matching service_uid is found.
  /// @remarks
  ///   The method filters out all services that have already been added,
  ///   then calls on_found_remote_service to ask the owner whether to
  ///   accept the remote service, and finally opens a new Tcp connection
  ///   which causes either on_connect_failed or on_connected to be called
  ///   later on.
  virtual bool add_service_provider(
    guid_t remote_host_guid, guid_t remote_service_guid,
    interface_uids_t remote_interface_uids, service_tag_t remote_service_tag,
    boost::asio::ip::address local_address,
    boost::asio::ip::tcp::endpoint remote_endpoint);

  /// @see service::sender.
  virtual std::pair<guid_t, guid_t> sender() const = 0;

  /// @see service::sender.
  virtual boost::asio::ip::address sender_address() const = 0;

  /// @see service::sender.
  virtual std::uint16_t sender_port() const = 0;

  /// @see service::broadcast.
  virtual void broadcast(const serialization::message& message) = 0;

  /// @see service::reply.
  virtual void reply(const serialization::message& message) = 0;

  /// @see service::send_to.
  virtual void send_to(guid_t remote_service_guid,
                       const serialization::message& message) = 0;

  /// @see service::send_to.
  virtual void send_to(const boost::asio::ip::udp::endpoint& target_endpoint,
                       const serialization::message& message) = 0;

  /// @see service::send_to.
  virtual void send_to_except(guid_t remote_service_guid,
                              const serialization::message& message) = 0;

  /// Read all available messages from the passed buffer.
  /// @param stream
  ///   When set this parameter to true messages may be split between
  ///   multiple buffers.
  void read_messages(std::vector<char> buffer, std::vector<char>& read_buffer,
                     serialization::protocol_version_t protocol_version,
                     bool stream);

  /// @see service::connected_services_count.
  virtual std::size_t connected_services_count() = 0;

  /// @see service::timeout.
  virtual std::chrono::high_resolution_clock::duration timeout() const = 0;

  /// @see service::timeout.
  virtual void timeout(
    std::chrono::high_resolution_clock::duration duration) = 0;

public:
  basic_service& service;
  bool started = false;
  interface_uids_t interface_uids;
  service_uid_t service_uid = 0;
  guid_t service_guid;
  bool is_provider = false;
  service_tag_t service_tag = 0;

  std::uint16_t service_port = 0;
  boost::asio::ip::address multicast_address;
  bool broadcast_service = true;
  bool use_handshake = true;

  std::unique_ptr<message_envelope> envelope;
};
}

#endif
