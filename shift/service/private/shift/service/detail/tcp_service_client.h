#ifndef SHIFT_SERVICE_DETAIL_TCPSERVICECLIENT_H
#define SHIFT_SERVICE_DETAIL_TCPSERVICECLIENT_H

#include <memory>
#include <functional>
#include <shift/network/network.h>
#include "shift/service/types.h"
#include "shift/service/service_host.h"
#include "shift/service/detail/tcp_service.h"

namespace shift::service::detail
{
/// Base class for service clients.
class tcp_service_client final : public tcp_service
{
public:
  /// Constructor.
  tcp_service_client(basic_service& service);

  /// Destructor.
  ~tcp_service_client() override;

  /// @see service::add_service_provider.
  bool add_service_provider(
    guid_t remote_host_guid, guid_t remote_service_guid,
    interface_uids_t remote_interface_uids, service_tag_t remote_service_tag,
    boost::asio::ip::address local_address,
    boost::asio::ip::tcp::endpoint remote_endpoint) override;

  /// The attempt to connect to the remote service has failed for whatever
  /// reason.
  void on_connect_failed(network::tcp_socket_client& socket,
                         boost::system::error_code error);

  /// This method gets called when the connection to a service provider has
  /// been established.
  /// @remarks
  ///   The method sends the initial handshake message, which is used to
  ///   validate that the two services actually match (using their
  ///   service_uid, serviceGuids and hostGuids) and to agree on a
  ///   protocol version supported by both ends. The answer will be received
  ///   using on_receive_handshake. If the remote service rejects the
  ///   connect
  ///   attempt it will close the connection and thus result in a call to
  ///   onDisconnected.
  void on_connected(network::tcp_socket_client& socket);

  /// This method gets called for the first received message, which is the
  /// handshake.
  /// @remarks
  ///   Once the handshake is received and accepted, the on_receive event is
  ///   re-bound to service::on_receive.
  void on_receive_handshake(network::tcp_socket_client& socket,
                            std::vector<char> buffer);
};
}

#endif
