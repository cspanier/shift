#ifndef SHIFT_PROTO_MESSAGE_HPP
#define SHIFT_PROTO_MESSAGE_HPP

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
#include "shift/proto/type_reference.hpp"
#include "shift/proto/node.hpp"

namespace shift::proto
{
struct field final : public node
{
  field() = default;
  field(const field&) = default;
  field(field&&) = default;
  ~field() override;
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
  ~message() override;
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
