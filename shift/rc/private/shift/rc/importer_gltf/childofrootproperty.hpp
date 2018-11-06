#ifndef SHIFT_RC_IMPORTER_GLTF_CHILDOFROOTPROPERTY_HPP
#define SHIFT_RC_IMPORTER_GLTF_CHILDOFROOTPROPERTY_HPP

#include <string>
#include "shift/rc/importer_gltf/types.hpp"
#include "shift/rc/importer_gltf/property.hpp"

namespace shift::rc::gltf
{
/// Child of a glTF root property.
struct child_of_root_property : public property
{
  /// Constructor.
  child_of_root_property(const parser::json::object& json_object);

  /// The user-defined name of this object. This is not necessarily unique,
  /// e.g., an accessor and a buffer could have the same name, or two
  /// accessors could even have the same name.
  std::string name;
};
}

#endif
