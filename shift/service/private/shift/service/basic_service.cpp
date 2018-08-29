#define BOOST_BIND_NO_PLACEHOLDERS
#include "shift/service/basic_service.h"
#include "shift/service/detail/service_impl.h"
#include "shift/service/detail/tcp_service_client.h"
#include "shift/service/detail/tcp_service_provider.h"
#include "shift/service/detail/udp_service.h"
#include "shift/service/service_host.h"
#include "shift/service/message_envelope.h"
#include <shift/serialization/archive.h>
#include <shift/serialization/message.h>
#include <shift/log/log.h>
#include <iomanip>

namespace shift::service
{
basic_service::basic_service(service_uid_t service_uid,
                             interface_uids_t interface_uids,
                             service_protocol protocol, bool is_provider,
                             service_tag_t service_tag)
: _protocol(protocol)
{
  switch (protocol)
  {
  case service_protocol::tcp:
    if (is_provider)
      _impl = std::make_unique<detail::tcp_service_provider>(*this);
    else
      _impl = std::make_unique<detail::tcp_service_client>(*this);
    break;

  case service_protocol::udp:
    _impl = std::make_unique<detail::udp_service>(*this, false);
    break;

  case service_protocol::udp_stream:
    _impl = std::make_unique<detail::udp_service>(*this, true);
    break;

  default:
    BOOST_ASSERT(false);
    return;
  }

  _impl->service_uid = service_uid;
  _impl->interface_uids = interface_uids;
  _impl->is_provider = is_provider;
  _impl->service_tag = service_tag;
}

basic_service::~basic_service()
{
  stop();
}

guid_t basic_service::service_guid() const
{
  return _impl->service_guid;
}

service_uid_t basic_service::service_uid() const
{
  return _impl->service_uid;
}

interface_uids_t basic_service::interface_uids() const
{
  return _impl->interface_uids;
}

service_protocol basic_service::protocol() const
{
  return _protocol;
}

bool basic_service::is_provider() const
{
  return _impl->is_provider;
}

service_tag_t basic_service::service_tag() const
{
  return _impl->service_tag;
}

void basic_service::service_port(std::uint16_t port)
{
  _impl->service_port = port;
}

std::uint16_t basic_service::service_port(
  boost::asio::ip::address bind_address) const
{
  return _impl->specific_service_port(std::move(bind_address));
}

void basic_service::multicast_address(boost::asio::ip::address address)
{
  if (!address.is_multicast())
  {
    log::error() << "The IP address " << address
                 << " is not a multicast address.";
    return;
  }
  _impl->multicast_address = address;
}

boost::asio::ip::address basic_service::multicast_address() const
{
  return _impl->multicast_address;
}

void basic_service::timeout(
  std::chrono::high_resolution_clock::duration duration)
{
  _impl->timeout(duration);
}

std::chrono::high_resolution_clock::duration basic_service::timeout() const
{
  return _impl->timeout();
}

void basic_service::broadcast_service(bool broadcast)
{
  _impl->broadcast_service = broadcast;
}

bool basic_service::broadcast_service() const
{
  return _impl->broadcast_service;
}

void basic_service::use_handshake(bool enable)
{
  _impl->use_handshake = enable;
}

bool basic_service::use_handshake() const
{
  return _impl->use_handshake;
}

void basic_service::envelope(std::unique_ptr<message_envelope> envelope)
{
  _impl->envelope = std::move(envelope);
}

std::size_t basic_service::connected_services_count() const
{
  return _impl->connected_services_count();
}

bool basic_service::start()
{
  BOOST_ASSERT(!_impl->started);
  if (_impl->started)
    return true;
  if (!_impl->start())
    return false;
  if (_impl->broadcast_service)
    service_host::singleton_instance().register_service(*this);
  _impl->started = true;
  return true;
}

void basic_service::stop()
{
  _impl->started = false;
  if (_impl->broadcast_service)
    service_host::singleton_instance().deregister_service(*this);
  _impl->stop();
}

std::pair<guid_t, guid_t> basic_service::sender() const
{
  return _impl->sender();
}

guid_t basic_service::sender_host_guid() const
{
  return _impl->sender().first;
}

guid_t basic_service::sender_service_guid() const
{
  return _impl->sender().second;
}

boost::asio::ip::address basic_service::sender_address() const
{
  return _impl->sender_address();
}

std::uint16_t basic_service::sender_port() const
{
  return _impl->sender_port();
}

void basic_service::broadcast(const serialization::message& message)
{
  _impl->broadcast(message);
}

void basic_service::reply(const serialization::message& message)
{
  _impl->reply(message);
}

void basic_service::send_to(guid_t remote_service_guid,
                            const serialization::message& message)
{
  _impl->send_to(remote_service_guid, message);
}

void basic_service::send_to(boost::asio::ip::udp::endpoint target_endpoint,
                            const serialization::message& message)
{
  _impl->send_to(target_endpoint, message);
}

void basic_service::send_to_except(guid_t remote_service_guid,
                                   const serialization::message& message)
{
  _impl->send_to_except(remote_service_guid, message);
}

bool basic_service::add_service_provider(
  guid_t remote_host_guid, guid_t remote_service_guid,
  interface_uids_t remote_interface_uids, service_tag_t remote_service_tag,
  boost::asio::ip::address local_address,
  boost::asio::ip::tcp::endpoint remote_endpoint)
{
  return _impl->add_service_provider(
    remote_host_guid, remote_service_guid, remote_interface_uids,
    remote_service_tag, std::move(local_address), std::move(remote_endpoint));
}
}
