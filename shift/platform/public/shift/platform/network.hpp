#ifndef SHIFT_PLATFORM_NETWORK_HPP
#define SHIFT_PLATFORM_NETWORK_HPP

#include <vector>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/asio/ip/address.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::platform
{
std::vector<boost::asio::ip::address> network_interfaces();
}

#endif
