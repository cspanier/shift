#ifndef SHIFT_NETWORK_SOCKETBASE_H
#define SHIFT_NETWORK_SOCKETBASE_H

#include <memory>
#include <shift/core/boost_disable_warnings.h>
#include <boost/asio/ip/address.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <boost/asio/ip/udp.hpp>
/// ToDo: Boost asio pulls Windows.h, which defines a lot of retarded macros...
///       Check how to stop boost from doing that.
#ifdef GetClassName
#undef GetClassName
#endif
#ifdef GetObject
#undef GetObject
#endif
#ifdef GetCommandLine
#undef GetCommandLine
#endif
#include <shift/log/log.h>
#include <shift/core/bit_field.h>
#include "shift/network/network.h"

namespace shift::network
{
using network_log = log::log_line<-10>;

///
using on_connected_t = std::function<void(tcp_socket_client& socket)>;

///
using on_connect_failed_t = std::function<void(
  tcp_socket_client& socket, boost::system::error_code error)>;

///
using on_closed_t =
  std::function<void(socket_base& socket, boost::system::error_code error)>;

///
using on_receive_packet_t =
  std::function<void(tcp_socket_client& socket, std::vector<char> buffer)>;

///
using on_receive_datagram_t =
  std::function<void(udp_socket& socket, boost::asio::ip::udp::endpoint sender,
                     std::vector<char> buffer)>;

/// socket behaviour flags.
enum class option
{
  /// The following flags specify whether Tcp sockets encode binary size
  /// information in the header of each packet. This enables the network
  /// library to deliver full messages rather than potentially fragmented
  /// ones (i.e. work packet based rather than stream based).
  write_packet8 = 1,
  write_packet16 = 2,
  write_packet32 = 4,
  write_packet_mask = write_packet8 | write_packet16 | write_packet32,
  read_packet8 = 8,
  read_packet16 = 16,
  read_packet32 = 32,
  read_packet_mask = read_packet8 | read_packet16 | read_packet32,

  /// If this flag is set Nagle algorithm for Tcp sockets is disabled.
  no_delay = 64
};

using options = core::bit_field<option>;

inline options operator|(option lhs, option rhs)
{
  return options{lhs} | rhs;
}

/// Public base class for all sockets.
class socket_base
{
public:
  /// Returns the associated user pointer.
  void* user_data() const;

  /// Assigns a new user pointer.
  void user_data(void* pointer);

  bool debug_message_payload = false;

protected:
  /// Constructor.
  socket_base() = default;

  void* _user_data = nullptr;
};
}

#endif
