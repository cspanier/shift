#ifndef SHIFT_RC_IMPORTER_GLTF_PRIMITIVEATTRIBUTE_HPP
#define SHIFT_RC_IMPORTER_GLTF_PRIMITIVEATTRIBUTE_HPP

#include <cstdint>
#include <map>
#include <string>
#include "shift/rc/importer_gltf/types.hpp"

namespace shift::rc::gltf
{
///
struct primitive_attribute
{
  /// Constructor.
  primitive_attribute(std::string accessor_id);

  /// The ID of the accessor containing an attribute.
  std::string accessor_id;
  /// The accessor referenced by accessor_id.
  gltf::accessor* accessor = nullptr;
};
}

#endif
