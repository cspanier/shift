#ifndef SHIFT_SERVICE_SERVICEHOST_H
#define SHIFT_SERVICE_SERVICEHOST_H

#include <cstdint>
#include <memory>
#include <vector>
#include <chrono>
#include <shift/core/boost_disable_warnings.h>
#include <boost/asio/ip/address.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/singleton.h>
#include <shift/serialization/types.h>
#include "shift/service/types.h"

namespace shift::service
{
/// The central service host class which takes care of basic network
/// communication to find other service hosts on the local network.
/// @remarks
///   Because the service library is based on the network library, you need
///   to instantiate the network::network_host singleton before creating this
///   one. Also note that you need to regularly call network_host::receive in
///   order to process messages received from remote services.
class service_host final
: public core::singleton<service_host, core::create::using_new>
{
public:
  /// Default constructor.
  service_host();

  /// Destructor.
  ~service_host();

  ///
  void bind_addresses(std::vector<boost::asio::ip::address> addresses);

  ///
  void multicast_addresses(boost::asio::ip::address_v4 multicast_address4,
                           boost::asio::ip::address_v6 multicast_address6);

  /// Starts the service host.
  bool start(serialization::protocol_version_t protocol_version,
             serialization::protocol_version_t required_version);

  /// Orders the service host to stop operation.
  void stop();

  /// Returns true until the service host is ordered to stop.
  bool running() const;

  /// Publish service information across the network. This method has to be
  /// called on a regular basis to make different instances find each other.
  void publish();

  /// Returns the list of local address which are used for service
  /// communication.
  const std::vector<boost::asio::ip::address>& bind_addresses() const;

  /// Returns the addresses being used for service discovery.
  std::pair<const boost::asio::ip::address_v4&,
            const boost::asio::ip::address_v6&>
  multicast_addresses() const;

  /// Returns the protocol version initially set when the host was started.
  serialization::protocol_version_t protocol_version() const;

  /// Returns the local host's GUID.
  guid_t guid() const;

  /// Overwrites the local host's GUID.
  void guid(guid_t new_guid);

  bool debug_multicasts = false;
  bool debug_handshakes = false;
  bool debug_service_registration = false;
  bool debug_service_calls = false;

  /// Registers a service.
  void register_service(basic_service& service);

  /// Deregisters a service.
  void deregister_service(basic_service& service);

  class impl;

private:
  std::unique_ptr<impl> _impl;

  friend class impl;
};
}

#endif
