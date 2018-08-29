#ifndef SHIFT_PROTO_ENUMERATION_H
#define SHIFT_PROTO_ENUMERATION_H

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
#include "shift/proto/node.h"
#include "shift/proto/type_reference.h"

namespace shift::proto
{
struct enumerator final : public node
{
  enumerator() = default;
  enumerator(const enumerator&) = default;
  enumerator(enumerator&&) = default;
  virtual ~enumerator() override;
  enumerator& operator=(const enumerator&) = default;
  enumerator& operator=(enumerator&&) = default;

  /// @see node::generate_uids.
  void generate_uids() override;

  std::int64_t value;
};
using enumerators = std::vector<enumerator>;

struct enumeration : public node
{
  enumeration() = default;
  enumeration(const enumeration&) = default;
  enumeration(enumeration&&) = default;
  virtual ~enumeration() override;
  enumeration& operator=(const enumeration&) = default;
  enumeration& operator=(enumeration&&) = default;

  /// @see node::generate_uids.
  void generate_uids() override;

  type_reference base;
  enumerators members;
};
}

#endif
