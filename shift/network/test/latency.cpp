#include <shift/network/network_host.h>
#include <shift/network/udp_socket.h>
#include <shift/log/log.h>
#include <shift/log/log_server.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <random>

using namespace shift;
using namespace shift::network;
using namespace boost::asio;
using namespace std::chrono;

template <typename T>
void postValueTo(udp_socket& socket, const ip::udp::endpoint& target, T value)
{
  std::vector<char> message;
  message.resize(sizeof(T));
  *reinterpret_cast<T*>(message.data()) = value;
  socket.post(std::move(message), target);
}

void postTimestampTo(udp_socket& socket, const ip::udp::endpoint& target)
{
  auto now = high_resolution_clock::now();
  postValueTo(socket, target,
              duration_cast<nanoseconds>(now.time_since_epoch()).count());
}

BOOST_AUTO_TEST_CASE(latency_ping_pong)
{
  auto& logServer = log::log_server::singleton_create();
  logServer.add_console_sink();

  const auto multicast_address4 = ip::address::from_string("239.255.255.5");
  const auto multicast_address6 = ip::address::from_string("FF05::74:A1E5:1");
  const std::uint16_t port = 0x1337;
  const ip::udp::endpoint multicastEndpoint4{multicast_address4, port};
  const ip::udp::endpoint multicastEndpoint6{multicast_address6, port};

  using guid_t = std::uint64_t;
  std::random_device random_device;
  std::mt19937_64 random_engine{random_device()};
  std::uniform_int_distribution<guid_t> random_distribution;
  const guid_t localGuid = random_distribution(random_engine);

  struct Statistics
  {
    Statistics(std::shared_ptr<udp_socket> socket, nanoseconds time)
    : socket(std::move(socket)),
      count(1),
      avgTime(time),
      minTime(time),
      maxTime(time)
    {
    }

    std::shared_ptr<udp_socket> socket;
    std::size_t count;
    nanoseconds avgTime;
    nanoseconds minTime;
    nanoseconds maxTime;
  };
  std::map<ip::udp::endpoint, Statistics> statisticsMap;
  std::mutex statisticsMutex;

  auto& network_host = network_host::singleton_create();
  network_host.debug_message_payload = true;
  network_host.start(1);

  std::cout << "Finding master instance..." << std::endl;
  bool isMaster = true;
  guid_t masterGuid = localGuid;
  ip::udp::endpoint masterEndpoint = multicastEndpoint4;

  std::vector<std::shared_ptr<udp_socket>> sockets;
  for (const auto& interface : network_host.local_interfaces())
  {
    auto multicast_address =
      interface.is_v4() ? multicast_address4 : multicast_address6;
    auto socket = std::make_shared<udp_socket>();
    if (!socket->open(interface, port, multicast_address))
    {
      continue;
    }
    socket->on_receive = [&](udp_socket& socket,
                             boost::asio::ip::udp::endpoint sender,
                             std::vector<char> buffer) {
      if (buffer.size() < sizeof(guid_t))
        return;
      guid_t remoteGuid = *reinterpret_cast<const guid_t*>(buffer.data());

      if (masterGuid > remoteGuid)
      {
        masterGuid = remoteGuid;
        masterEndpoint = sender;
        std::cout << "New master instance " << std::hex << std::setw(16)
                  << std::setfill('0') << remoteGuid << " is located at "
                  << sender << "." << std::dec << std::endl;
        isMaster = false;
      }
      else if (isMaster)
      {
        auto statisticsIterator = statisticsMap.find(sender);
        if (statisticsIterator == statisticsMap.end())
        {
          std::cout << "New slave instance " << std::hex << std::setw(16)
                    << std::setfill('0') << remoteGuid << " located at "
                    << sender << "." << std::dec << std::endl;
          statisticsMap.insert(std::make_pair(
            sender, Statistics{socket.shared_from_this(), milliseconds{20}}));
        }
      }
    };
    sockets.emplace_back(std::move(socket));
  }

  for (auto i = 0; i != 10 && isMaster; ++i)
  {
    for (auto& socket : sockets)
    {
      if (socket->local_endpoint().address().is_v4())
        postValueTo(*socket, multicastEndpoint4, localGuid);
      else
        postValueTo(*socket, multicastEndpoint6, localGuid);
    }
    network_host.receive(seconds(1));
  }

  if (isMaster)
    std::cout << "Start sending pings..." << std::endl;
  else
    std::cout << "Start listening for pings..." << std::endl;

  for (auto& socket : sockets)
  {
    socket->on_receive = [&](udp_socket& socket,
                             boost::asio::ip::udp::endpoint sender,
                             std::vector<char> buffer) {
      if (buffer.size() < sizeof(std::uint64_t))
        return;
      if (isMaster)
      {
        auto time =
          high_resolution_clock::now() -
          high_resolution_clock::time_point(
            duration_cast<high_resolution_clock::duration>(nanoseconds(
              *reinterpret_cast<const std::uint64_t*>(buffer.data()))));

        std::lock_guard lock(statisticsMutex);
        auto statisticsIterator = statisticsMap.find(sender);
        if (statisticsIterator != statisticsMap.end())
        {
          auto& statistics = statisticsIterator->second;
          statistics.avgTime = (statistics.avgTime * statistics.count + time) /
                               (statistics.count + 1);
          if (statistics.minTime > time)
            statistics.minTime = time;
          if (statistics.maxTime < time)
            statistics.maxTime = time;
          ++statistics.count;
        }
        else
        {
          statisticsMap.insert(std::make_pair(
            sender, Statistics{socket.shared_from_this(), time}));
        }
      }
      else
        socket.post(std::move(buffer), sender);
    };
  }

  std::atomic<bool> quit = ATOMIC_VAR_INIT(false);
  std::thread receiveThread([&]() {
    while (!quit)
    {
      network_host.receive(seconds(10));
      if (isMaster)
      {
        std::lock_guard lock(statisticsMutex);
        for (const auto& statisticsPair : statisticsMap)
        {
          const auto& statistics = statisticsPair.second;
          std::cout << statisticsPair.first << "  count: " << statistics.count
                    << "  avg: "
                    << duration_cast<milliseconds>(statistics.avgTime).count()
                    << "ms - min: "
                    << duration_cast<milliseconds>(statistics.minTime).count()
                    << "ms - max: "
                    << duration_cast<milliseconds>(statistics.maxTime).count()
                    << "ms" << std::endl;
        }
        std::cout << "-----" << std::endl;
      }
      else
        std::cout << "..." << std::endl;
    }
  });
  std::thread sendThread([&]() {
    while (!quit && isMaster)
    {
      {
        std::lock_guard lock(statisticsMutex);
        for (const auto& statisticsPair : statisticsMap)
          postTimestampTo(*statisticsPair.second.socket, statisticsPair.first);
      }
      std::this_thread::sleep_for(milliseconds(20));
    }
  });

  std::this_thread::sleep_for(seconds(60));
  quit = true;
  receiveThread.join();
  sendThread.join();

  statisticsMap.clear();
  for (auto& socket : sockets)
    socket->close();
  sockets.clear();
  network_host.stop();
}
