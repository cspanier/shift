#ifndef SHIFT_SERVICE_DETAIL_UDPSERVICEBASE_H
#define SHIFT_SERVICE_DETAIL_UDPSERVICEBASE_H

#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>
#include <shift/core/boost_disable_warnings.h>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ip/address.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <boost/asio/ip/udp.hpp>
#include <shift/serialization/message.h>
#include <shift/network/network.h>
#include "shift/service/types.h"
#include "shift/service/detail/service_impl.h"

namespace shift::service::detail
{
struct udp_connection;

/// Base class for TCP services.
class udp_service final : public service_impl
{
public:
  /// Constructor.
  udp_service(basic_service& service, bool stream);

  /// Destructor.
  ~udp_service() override;

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
  void on_receive(network::udp_socket& socket,
                  const boost::asio::ip::udp::endpoint& sender,
                  std::vector<char> buffer);

  /// @see service::add_service_provider.
  bool add_service_provider(
    guid_t remote_host_guid, guid_t remote_service_guid,
    interface_uids_t remote_interface_uids, service_tag_t remote_service_tag,
    boost::asio::ip::address local_address,
    boost::asio::ip::tcp::endpoint remote_endpoint) override;

  /// This method gets called each time the _timeoutTimer times out.
  void check_disconnects(const boost::system::error_code& error);

  /// @see service::connected_services_count.
  std::size_t connected_services_count() override;

  /// @see service::timeout.
  std::chrono::high_resolution_clock::duration timeout() const override;

  /// @see service::timeout.
  void timeout(std::chrono::high_resolution_clock::duration duration) override;

  bool _stream;
  std::vector<std::shared_ptr<network::udp_socket>> _sockets;
  std::recursive_mutex _connection_mutex;
  std::vector<std::shared_ptr<udp_connection>> _connections;
  std::chrono::high_resolution_clock::duration _timeout =
    std::chrono::seconds(0);
  boost::asio::deadline_timer _timeoutTimer;

  static thread_local udp_connection* _caller;
};
}

#endif
