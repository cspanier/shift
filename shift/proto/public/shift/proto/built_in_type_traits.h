#ifndef SHIFT_PROTO_BUILTINTYPETRAITS_H
#define SHIFT_PROTO_BUILTINTYPETRAITS_H

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

namespace shift::proto
{
///
struct built_in_type_traits
{
  ///
  static bool is_char(built_in_type type);

  ///
  static bool is_signed_int(built_in_type type);

  ///
  static bool is_unsigned_int(built_in_type type);

  ///
  static bool is_int(built_in_type type);

  ///
  static bool is_float(built_in_type type);

  ///
  static bool is_string(built_in_type type);

  ///
  static bool is_template(built_in_type type);
};
}

#endif
