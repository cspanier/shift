#include "shift/network/tcp_socket_listener.h"
#include "shift/network/tcp_socket_listener_impl.h"
#include "shift/network/network_host.h"
#include <shift/log/log.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <functional>

namespace shift::network
{
tcp_socket_listener::tcp_socket_listener(network::options options)
: _impl(std::make_unique<impl>(options))
{
  BOOST_ASSERT(!_impl->acceptor.is_open());
}

tcp_socket_listener::~tcp_socket_listener()
{
  BOOST_ASSERT(network_host::singleton_instantiated());
  if (_impl->host.debug_socket_lifetime)
    network_log() << "Deleting Tcp listener socket.";
}

void tcp_socket_listener::close()
{
  _impl->close();
}

bool tcp_socket_listener::listen(boost::asio::ip::address bind_address,
                                 std::uint16_t bind_port)
{
  return _impl->listen(*this, bind_address, bind_port);
}

boost::asio::ip::tcp::endpoint tcp_socket_listener::local_endpoint() const
{
  return _impl->acceptor.local_endpoint();
}
}
