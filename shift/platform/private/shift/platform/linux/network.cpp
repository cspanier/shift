#include "shift/platform/network.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <array>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <linux/if_link.h>

namespace shift::platform
{
std::vector<boost::asio::ip::address> network_interfaces()
{
  std::vector<boost::asio::ip::address> result;

  std::unique_ptr<ifaddrs, void (*)(ifaddrs*)> interfaces(nullptr,
                                                          &freeifaddrs);
  {
    ifaddrs* p = nullptr;
    if (getifaddrs(&p) == -1)
      return result;
    interfaces.reset(p);
  }

  std::array<char, NI_MAXHOST> interfaceBuffer;
  for (auto* interface = interfaces.get(); interface != nullptr;
       interface = interface->ifa_next)
  {
    if ((interface->ifa_addr == nullptr) ||
        (interface->ifa_ifu.ifu_broadaddr == nullptr))
      continue;

    auto family = interface->ifa_addr->sa_family;

    if (family == AF_INET || family == AF_INET6)
    {
      if (getnameinfo(interface->ifa_addr,
                      (family == AF_INET) ? sizeof(struct sockaddr_in)
                                          : sizeof(struct sockaddr_in6),
                      interfaceBuffer.data(), interfaceBuffer.size(), nullptr,
                      0, NI_NUMERICHOST) == 0)
      {
        result.push_back(
          boost::asio::ip::address::from_string(interfaceBuffer.data()));
      }
    }
  }
  return result;
}
}
