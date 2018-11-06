#include "shift/network/tcp_socket_client.hpp"
#include "shift/network/tcp_socket_client_impl.hpp"
#include "shift/network/network_host.hpp"
#include <shift/log/log.hpp>
#include <shift/platform/fpexceptions.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/bind.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::network
{
tcp_socket_client::tcp_socket_client(network::options options)
: _impl(std::make_unique<impl>(options))
{
}

tcp_socket_client::~tcp_socket_client()
{
  // If the following assertion fails you did a bad job in making sure that
  // all socket instances are being destroyed before the network host.
  BOOST_ASSERT(network_host::singleton_instantiated());

  if (_impl->underlying_socket.is_open())
    close();
  if (_impl->host.debug_socket_lifetime)
    network_log() << "Deleting Tcp client socket.";
}

std::shared_ptr<tcp_socket_listener> tcp_socket_client::parent() const
{
  return _impl->parent;
}

void tcp_socket_client::close()
{
  _impl->close(*this);
}

bool tcp_socket_client::connect(const boost::asio::ip::address& remote_address,
                                std::uint16_t remote_port)
{
  return _impl->connect(*this, remote_address, remote_port);
}

void tcp_socket_client::post(std::vector<char> buffer)
{
  // Serialize posting of new buffers with the coroutine.
  _impl->read_write_strand.post(
    boost::bind(&tcp_socket_client::impl::post_routine, _impl.get(),
                shared_from_this(), std::move(buffer)));
}

void tcp_socket_client::queue(std::vector<char> buffer)
{
  network_host::singleton_instance().queue_packet(shared_from_this(),
                                                  std::move(buffer));
}

boost::asio::ip::tcp::endpoint tcp_socket_client::local_endpoint() const
{
  return _impl->underlying_socket.local_endpoint();
}

boost::asio::ip::tcp::endpoint tcp_socket_client::remote_endpoint() const
{
  return _impl->underlying_socket.remote_endpoint();
}

void tcp_socket_client::parent(std::shared_ptr<tcp_socket_listener> other)
{
  _impl->parent = std::move(other);
}

void tcp_socket_client::adopt()
{
  _impl->adopt(*this);
}
}
