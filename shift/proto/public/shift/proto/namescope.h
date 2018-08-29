#ifndef SHIFT_PROTO_NAMESCOPE_H
#define SHIFT_PROTO_NAMESCOPE_H

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
#include <string_view>
#include <utility>
#include <shift/core/boost_disable_warnings.h>
#include <boost/lexical_cast.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/variant.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/proto/types.h"
#include "shift/proto/node.h"

namespace shift::proto
{
struct namescope : public node
{
  namescope() = default;
  namescope(const namescope&) = default;
  namescope(namescope&&) = default;
  virtual ~namescope() override;
  namescope& operator=(const namescope&) = default;
  namescope& operator=(namescope&&) = default;

  /// @see node::find_type.
  proto::type* find_type(const namescope_path& namescope_path,
                         std::size_t current_depth = 0) override;

  std::vector<proto::type> static_children;
  std::vector<std::shared_ptr<proto::type>> dynamic_children;

  std::vector<namescope*> namescopes;
  std::vector<alias*> aliases;
  std::vector<enumeration*> enumerations;
  std::vector<message*> messages;
  std::vector<interface*> interfaces;
  std::vector<service*> services;

  /// Parses a source document into this namescope.
  void parse(std::string_view content);

  /// Assigns the 'source_filename' attribute of all nodes which don't have it
  /// set, yet. This is particularly useful to track where a node is defined.
  void symbol_source(const std::string& source_path);

  /// Collapse all name scopes sharing the same name into a single one.
  void merge();

  /// Converts static_children to dynamic_children, because we want to store
  /// pointers to type and still need to be able to resize the type vectors
  /// (which might invalidate all pointers on reallocation).
  void convert_type_vectors();

  /// Unpacks all nested name scopes into a more handy structure.
  void unpack();

  /// Sorts all vectors by the name of their elements. You should call this
  /// after namescope::unpack to get stable results.
  void sort();

  /// Semantically check the collection of all nodes.
  bool analyze();

  /// @see node::generate_uids.
  void generate_uids() override;
};
}

#endif
