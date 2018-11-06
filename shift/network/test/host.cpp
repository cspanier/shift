#include <shift/network/network_host.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <iostream>

using namespace shift;
using namespace shift::network;
using namespace boost::asio;
using namespace std::chrono;

BOOST_AUTO_TEST_CASE(network_local_addresses)
{
  BOOST_ASSERT(!network_host::singleton_instantiated());
  auto& network_host = network_host::singleton_create();

  std::cout << "Local addresses:" << std::endl;
  for (const auto& interface : network_host.local_interfaces())
    std::cout << "\t" << interface << std::endl;

  network_host::singleton_destroy();
}

BOOST_AUTO_TEST_CASE(network_no_start)
{
  BOOST_ASSERT(!network_host::singleton_instantiated());
  auto& network_host = network_host::singleton_create();
  network_host.stop();
  network_host::singleton_destroy();
}

BOOST_AUTO_TEST_CASE(network_udp_socket)
{
  BOOST_ASSERT(!network_host::singleton_instantiated());
  auto& network_host = network_host::singleton_create();
  network_host.start(1);

  {
    auto socket = std::make_shared<udp_socket>();
    BOOST_CHECK(socket->open(ip::address::from_string("127.0.0.1"), 0x1337,
                             ip::address::from_string("127.0.0.1")));
    socket->close();
  }

  network_host.stop();
  network_host::singleton_destroy();
}

BOOST_AUTO_TEST_CASE(network_tcp_socket)
{
  BOOST_ASSERT(!network_host::singleton_instantiated());
  auto& network_host = network_host::singleton_create();
  network_host.start(1);

  {
    bool clientConnectedToListener = false;
    bool listenerAcceptedClient = false;

    auto listener =
      std::make_shared<tcp_socket_listener>(network::option::no_delay);
    std::shared_ptr<tcp_socket_client> listenerClient;
    listener->on_connected = [&](tcp_socket_client& socket) {
      BOOST_CHECK(!listenerAcceptedClient);
      listenerAcceptedClient = true;
      listenerClient = socket.shared_from_this();
    };
    listener->on_closed = [&](network::socket_base& /*socket*/,
                              boost::system::error_code /*error*/) {
      BOOST_CHECK(listenerAcceptedClient);
      listenerAcceptedClient = false;
      listenerClient.reset();
    };
    BOOST_CHECK(
      listener->listen(ip::address::from_string("127.0.0.1"), 0x1337));

    network_host.receive(milliseconds(200));

    auto client =
      std::make_shared<tcp_socket_client>(network::option::no_delay);
    client->on_connected = [&](tcp_socket_client& /*socket*/) {
      BOOST_CHECK(!clientConnectedToListener);
      clientConnectedToListener = true;
    };
    client->on_closed = [&](network::socket_base& /*socket*/,
                            boost::system::error_code /*error*/) {
      BOOST_CHECK(clientConnectedToListener);
      clientConnectedToListener = false;
    };
    BOOST_CHECK(client->connect(ip::address::from_string("127.0.0.1"), 0x1337));

    while (!clientConnectedToListener && !listenerAcceptedClient)
      network_host.receive(milliseconds(200));

    client->close();

    while (clientConnectedToListener || listenerAcceptedClient)
      network_host.receive(milliseconds(200));
  }

  network_host.stop();
  network_host::singleton_destroy();
}

BOOST_AUTO_TEST_CASE(network_singlethreaded)
{
  BOOST_ASSERT(!network_host::singleton_instantiated());
  auto& network_host = network_host::singleton_create();
  network_host.start(1);
  network_host.stop();
  network_host::singleton_destroy();
}

BOOST_AUTO_TEST_CASE(network_multithreaded)
{
  BOOST_ASSERT(!network_host::singleton_instantiated());
  auto& network_host = network_host::singleton_create();
  network_host.start(10);
  network_host.stop();
  network_host::singleton_destroy();
}
