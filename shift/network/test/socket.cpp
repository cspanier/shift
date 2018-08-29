#include <shift/network/network_host.h>
#include <shift/network/udp_socket.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

using namespace shift;
using namespace shift::network;
using namespace boost::asio;

BOOST_AUTO_TEST_CASE(socket_udp_no_close)
{
  auto& network_host = network_host::singleton_create();
  network_host.start(1);

  {
    auto socket = std::make_shared<udp_socket>();
    socket->open(ip::address::from_string("127.0.0.1"), 0x1337,
                 ip::address::from_string("127.0.0.1"));
    // socket->close();
  }

  network_host.stop();
  network_host::singleton_destroy();
}

BOOST_AUTO_TEST_CASE(socket_tcp_listener_no_close)
{
  auto& network_host = network_host::singleton_create();
  network_host.start(1);

  {
    auto listenerSocket = std::make_shared<tcp_socket_listener>(
      network::option::no_delay | network::option::read_packet32 |
      network::option::write_packet32);
    listenerSocket->listen(ip::address::from_string("127.0.0.1"), 0x1337);
    // socket->close();
  }

  network_host.stop();
  network_host::singleton_destroy();
}

BOOST_AUTO_TEST_CASE(socket_tcp_client_no_close)
{
  auto& network_host = network_host::singleton_create();
  network_host.start(1);

  {
    auto listenerSocket = std::make_shared<tcp_socket_listener>(
      network::option::no_delay | network::option::read_packet32 |
      network::option::write_packet32);
    listenerSocket->listen(ip::address::from_string("127.0.0.1"), 0x1337);
    auto clientSocket = std::make_shared<tcp_socket_client>(
      network::option::no_delay | network::option::read_packet32 |
      network::option::write_packet32);
    clientSocket->connect(ip::address::from_string("127.0.0.1"), 0x1337);
    // clientSocket->close();
    // listenerSocket->close();
  }

  network_host.stop();
  network_host::singleton_destroy();
}
