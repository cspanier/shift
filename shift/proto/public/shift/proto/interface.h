#ifndef SHIFT_PROTO_INTERFACE_H
#define SHIFT_PROTO_INTERFACE_H

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
#include <shift/core/boost_disable_warnings.h>
#include <boost/lexical_cast.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/variant.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/proto/types.h"
#include "shift/proto/namescope.h"

namespace shift::proto
{
struct interface final : public namescope
{
  interface() = default;
  interface(const interface&) = default;
  interface(interface&&) = default;
  ~interface() override;
  interface& operator=(const interface&) = default;
  interface& operator=(interface&&) = default;

  /// @see node::find_type.
  proto::type* find_type(const namescope_path& namescope_path,
                         std::size_t current_depth = 0) override;

  /// @see node::generate_uids.
  void generate_uids() override;
};
}

#endif
