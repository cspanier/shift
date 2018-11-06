#ifndef SHIFT_PROTO_TYPEREFERENCE_HPP
#define SHIFT_PROTO_TYPEREFERENCE_HPP

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <memory>
#include <variant>
#include <vector>
#include <string>
#include <utility>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/lexical_cast.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/proto/types.hpp"

namespace shift::proto
{
struct type_reference
{
  /// Returns a pointer to a built-in type if this type represents one,
  /// otherwise nullptr is returned.
  /// @param recursive
  ///   Resolve the alias recursively, in case it points to another alias.
  const proto::built_in_type* as_built_in_type(bool recursive = false) const;

  /// Returns a pointer to an alias type if this type represents one,
  /// otherwise nullptr is returned.
  const alias* as_alias() const;

  /// Returns a pointer to an enumeration type if this type represents one,
  /// otherwise nullptr is returned.
  /// @param recursive
  ///   Resolve the alias recursively, in case it points to another alias.
  const enumeration* as_enumeration(bool recursive = false) const;

  /// Returns a pointer to a message type if this type represents one,
  /// otherwise nullptr is returned.
  /// @param recursive
  ///   Resolve the alias recursively, in case it points to another alias.
  const message* as_message(bool recursive = false) const;

  std::variant<proto::built_in_type, type_path> name;
  template_arguments arguments;

  type_variant variant;
};
}

#endif
