#ifndef SHIFT_SERVICE_DETAIL_GUID_GENERATOR_HPP
#define SHIFT_SERVICE_DETAIL_GUID_GENERATOR_HPP

#include "shift/service/types.hpp"

namespace shift::service::detail
{
/// Generates a new (hopefully) globally unique random guid_t.
guid_t generate_guid();
}

#endif
