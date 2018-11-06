#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "shift/platform/network.hpp"
#include <winsock2.h>
#include <iphlpapi.h>

namespace shift::platform
{
std::vector<boost::asio::ip::address> network_interfaces()
{
  /// ToDo: We might need to call WSAStartup, which is usually already done by
  /// Boost asio.

  std::vector<boost::asio::ip::address> result;

  // Microsoft recommends this default buffer size.
  unsigned long adapter_info_buffer_size = 15 * 1024;
  std::vector<char> adapter_info_buffer;
  unsigned long error_code = 0;
  do
  {
    adapter_info_buffer.resize(adapter_info_buffer_size);
    error_code = GetAdaptersAddresses(
      AF_UNSPEC,
      GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
        GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME,
      nullptr,
      reinterpret_cast<PIP_ADAPTER_ADDRESSES>(adapter_info_buffer.data()),
      &adapter_info_buffer_size);
    if (error_code == ERROR_BUFFER_OVERFLOW)
      continue;
    else if (error_code == ERROR_SUCCESS)
      break;
    else
    {
      // An unknown error occured.
      return {};
    }
  } while (true);

  std::vector<char> ip_address;
  for (const auto* adapter_addresses =
         reinterpret_cast<PIP_ADAPTER_ADDRESSES>(adapter_info_buffer.data());
       adapter_addresses; adapter_addresses = adapter_addresses->Next)
  {
    // Skip local loopback devices/addresses.
    if (adapter_addresses->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
      continue;

    for (auto* address = adapter_addresses->FirstUnicastAddress; address;
         address = address->Next)
    {
      if (address->Flags == IP_ADAPTER_ADDRESS_TRANSIENT)
      {
        // The IP address is a cluster address and should not be used by most
        // applications.
        continue;
      }

      DWORD ip_address_size = 0;
      if (WSAAddressToString(
            reinterpret_cast<LPSOCKADDR>(address->Address.lpSockaddr),
            address->Address.iSockaddrLength, nullptr, nullptr,
            &ip_address_size) != SOCKET_ERROR ||
          WSAGetLastError() != WSAEFAULT)
      {
        // Unknown error. Skip this address.
        continue;
      }
      ip_address.resize(ip_address_size);
      if (WSAAddressToString(
            reinterpret_cast<LPSOCKADDR>(address->Address.lpSockaddr),
            address->Address.iSockaddrLength, nullptr, ip_address.data(),
            &ip_address_size) != 0)
      {
        // Unknown error. Skip this address.
        continue;
      }
      result.push_back(boost::asio::ip::address::from_string(
        std::string{ip_address.data(), ip_address_size}));
    }
  }
  return result;
}
}
