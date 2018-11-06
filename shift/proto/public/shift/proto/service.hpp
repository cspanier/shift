#ifndef SHIFT_PROTO_SERVICE_HPP
#define SHIFT_PROTO_SERVICE_HPP

#include <cstddef>
#include <cstdint>
#include <list>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/variant.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/proto/types.hpp"
#include "shift/proto/namescope.hpp"

namespace shift::proto
{
struct service final : public namescope
{
  /// @see node::generate_uids.
  void generate_uids() override;

  bool is_provider = false;
  interface* service_interface = nullptr;
  type_path service_name;
  interface* callback_interface = nullptr;
  type_path callback_name;
};
}

#endif
