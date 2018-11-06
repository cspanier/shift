#ifndef SHIFT_PROTO_ALIAS_HPP
#define SHIFT_PROTO_ALIAS_HPP

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
#include "shift/proto/node.hpp"
#include "shift/proto/type_reference.hpp"

namespace shift::proto
{
struct alias final : public node
{
  alias() = default;
  alias(const alias&) = default;
  alias(alias&&) = default;
  ~alias() override;
  alias& operator=(const alias&) = default;
  alias& operator=(alias&&) = default;

  /// @see node::generate_uids.
  void generate_uids() override;

  /// Returns the actual type referenced, which is different from
  /// type_reference when chaining aliases.
  const type_reference& actual_type_reference() const;

  type_reference reference;
};
}

#endif
