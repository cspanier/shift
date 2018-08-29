#ifndef SHIFT_SERVICE_DETAIL_GUID_GENERATOR_H
#define SHIFT_SERVICE_DETAIL_GUID_GENERATOR_H

#include "shift/service/types.h"

namespace shift::service::detail
{
/// Generates a new (hopefully) globally unique random guid_t.
guid_t generate_guid();
}

#endif
