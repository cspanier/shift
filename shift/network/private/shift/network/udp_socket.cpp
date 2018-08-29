#include "shift/network/udp_socket.h"
#include "shift/network/udp_socket_impl.h"
#include "shift/network/network_host.h"
#include <shift/log/log.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <functional>

namespace shift::network
{
udp_socket::udp_socket() : _impl(std::make_unique<impl>())
{
}

udp_socket::~udp_socket()
{
  // If the following assertion fails you did a bad job in making sure that
  // all socket instances are being destroyed before the network host.
  BOOST_ASSERT(network_host::singleton_instantiated());

  if (_impl->underlying_socket.is_open())
  {
    /// ToDo: Calling close from dtor doesn't work as it attempts to call
    /// this->shared_from_this(), which is no longer valid here.
    close();
  }
  if (_impl->host.debug_socket_lifetime)
    network_log() << "Deleting Udp socket.";
}

bool udp_socket::open(const boost::asio::ip::address& bind_address,
                      std::uint16_t bind_port,
                      const boost::asio::ip::address& multicast_address)
{
  return _impl->open(*this, bind_address, bind_port, multicast_address);
}

void udp_socket::close()
{
  _impl->close(*this);
}

void udp_socket::post(std::vector<char> buffer,
                      const boost::asio::ip::udp::endpoint& destination)
{
  // Serialize posting of new buffers with udp_socket::writeRoutine.
  _impl->read_write_strand.post(boost::bind(&udp_socket::impl::post,
                                            _impl.get(), shared_from_this(),
                                            std::move(buffer), destination));
}

void udp_socket::queue(std::vector<char> buffer,
                       const boost::asio::ip::udp::endpoint& sender)
{
  network_host::singleton_instance().queue_datagram(shared_from_this(),
                                                    std::move(buffer), sender);
}

boost::asio::ip::udp::endpoint udp_socket::local_endpoint() const
{
  return _impl->local_endpoint;
}
}
