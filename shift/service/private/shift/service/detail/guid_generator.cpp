#include "shift/service/detail/guid_generator.h"
#include <random>

namespace shift::service::detail
{
guid_t generate_guid()
{
  thread_local std::mt19937_64 random_engine{std::random_device{}()};

  return std::uniform_int_distribution<guid_t>{}(random_engine);
}
}
