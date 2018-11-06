#ifndef SHIFT_SERVICE_DETAIL_TCPSERVICEBASE_HPP
#define SHIFT_SERVICE_DETAIL_TCPSERVICEBASE_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>
#include <shift/serialization/message.hpp>
#include <shift/network/network.hpp>
#include "shift/service/types.hpp"
#include "shift/service/detail/service_impl.hpp"

namespace shift::service::detail
{
struct tcp_connection;

/// Base class for TCP services.
class tcp_service : public service_impl
{
public:
  /// Constructor.
  tcp_service(basic_service& service);

  /// Destructor.
  ~tcp_service() override;

  /// @see service_impl::start.
  bool start() override;

  /// @see service_impl::stop.
  void stop() override;

  /// @see service::sender.
  std::pair<guid_t, guid_t> sender() const override;

  /// @see service::sender.
  boost::asio::ip::address sender_address() const override;

  /// @see service::sender.
  std::uint16_t sender_port() const override;

  /// @see service::broadcast.
  void broadcast(const serialization::message& message) override;

  /// @see service::reply.
  void reply(const serialization::message& message) override;

  /// @see service::send_to.
  void send_to(guid_t remote_service_guid,
               const serialization::message& message) override;

  /// @see service::send_to.
  void send_to(const boost::asio::ip::udp::endpoint& target_endpoint,
               const serialization::message& message) override;

  /// @see service::send_to_except.
  void send_to_except(guid_t remote_service_guid,
                      const serialization::message& message) override;

  ///
  void on_receive(network::tcp_socket_client& socket, std::vector<char> buffer);

  /// Sends the initial handshake message.
  void send_handshake(network::tcp_socket_client& socket);

  /// Receives the initial handshake message.
  bool receive_handshake(network::tcp_socket_client& socket,
                         std::vector<char> buffer, bool reply_handshake);

  /// Moves a connection from _pending_connections to
  /// _established_connections.
  void establish_connection(tcp_connection* connection);

  /// This method gets called when the remote service client closes the
  /// connection.
  void on_closed(network::socket_base& socket, boost::system::error_code error);

  /// @see service::connected_services_count.
  std::size_t connected_services_count() override;

  /// @see service::timeout.
  std::chrono::high_resolution_clock::duration timeout() const override;

  /// @see service::timeout.
  void timeout(std::chrono::high_resolution_clock::duration duration) override;

  std::mutex _connection_mutex;
  std::vector<std::unique_ptr<tcp_connection>> _pending_connections;
  std::vector<std::unique_ptr<tcp_connection>> _established_connections;

  static thread_local tcp_connection* _caller;
};
}

#endif
