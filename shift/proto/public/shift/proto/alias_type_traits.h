#ifndef SHIFT_PROTO_ALIASTYPETRAITS_H
#define SHIFT_PROTO_ALIASTYPETRAITS_H

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
struct alias_type_traits
{
  ///
  static bool is_char(const alias& type);

  ///
  static bool is_signed_int(const alias& type);

  ///
  static bool is_unsigned_int(const alias& type);

  ///
  static bool is_int(const alias& type);

  ///
  static bool is_float(const alias& type);

  ///
  static bool is_string(const alias& type);

  ///
  static bool is_template(const alias& type);
};
}

#endif
