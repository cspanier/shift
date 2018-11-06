#ifndef SHIFT_NETWORK_NETWORK_HPP
#define SHIFT_NETWORK_NETWORK_HPP

#include <cstddef>
#include <cstdint>
#include <memory>

#if defined(SHIFT_NETWORK_SHARED)
#if defined(SHIFT_NETWORK_EXPORT)
#define SHIFT_NETWORK_API SHIFT_EXPORT
#else
#define SHIFT_NETWORK_API SHIFT_IMPORT
#endif
#else
#define SHIFT_NETWORK_API
#endif

namespace shift::network
{
class network_host;

class socket_base;
class udp_socket;
class tcp_socket_client;
class tcp_socket_listener;
}

#endif
