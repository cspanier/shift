#ifndef SHIFT_SERVICE_TYPES_H
#define SHIFT_SERVICE_TYPES_H

#include <cstdint>
#include <memory>
#include <shift/log/log.h>

namespace shift::service
{
using guid_t = std::uint64_t;
using interface_uid_t = std::uint32_t;
using interface_uids_t = std::pair<interface_uid_t, interface_uid_t>;
using service_uid_t = std::uint32_t;
using service_tag_t = std::uint16_t;

class service_host;
class basic_service;
class message_envelope;
}

#endif
