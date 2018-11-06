#include "shift/network/socket_base.hpp"
#include "shift/network/network_host.hpp"

namespace shift::network
{
void* socket_base::user_data() const
{
  return _user_data;
}

void socket_base::user_data(void* pointer)
{
  _user_data = pointer;
}
}
