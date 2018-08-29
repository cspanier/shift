#ifndef SHIFT_PROTO_MESSAGE_H
#define SHIFT_PROTO_MESSAGE_H

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
#include "shift/proto/type_reference.h"
#include "shift/proto/node.h"

namespace shift::proto
{
struct field final : public node
{
  field() = default;
  field(const field&) = default;
  field(field&&) = default;
  virtual ~field() override;
  field& operator=(const field&) = default;
  field& operator=(field&&) = default;

  /// @see node::generate_uids.
  void generate_uids() override;

  type_reference reference;
};

using fields_t = std::vector<field>;

struct message final : public node
{
  message() = default;
  message(const message&) = default;
  message(message&&) = default;
  virtual ~message() override;
  message& operator=(const message&) = default;
  message& operator=(message&&) = default;

  /// @see node::generate_uids.
  void generate_uids() override;

  /// Returns whether this message, or one of the bases, has any fields.
  bool has_fields() const;

  message* base = nullptr;
  type_path base_name;
  fields_t fields;
};
}

#endif
