#ifndef SHIFT_PLATFORM_NETWORK_H
#define SHIFT_PLATFORM_NETWORK_H

#include <vector>
#include <shift/core/boost_disable_warnings.h>
#include <boost/asio/ip/address.hpp>
#include <shift/core/boost_restore_warnings.h>

namespace shift::platform
{
std::vector<boost::asio::ip::address> network_interfaces();
}

#endif
