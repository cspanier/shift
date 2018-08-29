#ifndef SHIFT_SERVICE_DETAIL_TCPSERVICEPROVIDER_H
#define SHIFT_SERVICE_DETAIL_TCPSERVICEPROVIDER_H

#include <memory>
#include <vector>
#include <functional>
#include <shift/network/network.h>
#include "shift/service/types.h"
#include "shift/service/service_host.h"
#include "shift/service/detail/tcp_service.h"

namespace shift::service::detail
{
/// Base class for service providers.
class tcp_service_provider final : public tcp_service
{
public:
  /// Constructor.
  tcp_service_provider(basic_service& service);

  /// Destructor.
  ~tcp_service_provider() override;

  /// @see service_impl::service_port.
  std::uint16_t specific_service_port(
    boost::asio::ip::address bind_address) const override;

  /// @see service_impl::start.
  bool start() override;

  /// @see service_impl::stop.
  void stop() override;

private:
  /// This method gets called when a new service client connects to this
  /// provider.
  /// @remarks
  ///   The new connection will not be active until the handshake is
  ///   completed, which will be initiated by the client.
  void on_connected(network::tcp_socket_client& socket);

  /// This method gets called for the first received message, which is the
  /// handshake.
  /// @remarks
  ///   Once the handshake is received and accepted, the on_receive event is
  ///   re-bound to service::on_receive. The service provider also sends
  ///   back its own handshake to acknowledge it.
  void on_receive_handshake(network::tcp_socket_client& socket,
                            std::vector<char> buffer);

  std::vector<std::shared_ptr<network::tcp_socket_listener>> _listeners;
};
}

#endif
