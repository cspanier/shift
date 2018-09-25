#ifndef SHIFT_RC_IMPORTER_GLTF_MATERIAL_H
#define SHIFT_RC_IMPORTER_GLTF_MATERIAL_H

#include <memory>
#include <variant>
#include <string>
#include <vector>
#include <shift/math/vector.h>
#include "shift/rc/importer_gltf/types.h"
#include "shift/rc/importer_gltf/childofrootproperty.h"

namespace shift::rc::gltf
{
using material_value =
  std::variant<std::int32_t, float, std::string, math::vector2<std::int32_t>,
               math::vector3<std::int32_t>, math::vector4<std::int32_t>,
               math::vector2<float>, math::vector3<float>,
               math::vector4<float>>;

///
struct material : public child_of_root_property
{
  /// Constructor.
  material(const parser::json::object& json_object);

  /// The ID of the technique. If this is not supplied, and no extension is
  /// present that defines material properties, then the primitive should be
  /// rendered using a default material with 50% gray emissive color.
  std::string technique_id;
  /// The technique referenced by technique_id.
  gltf::technique* technique = nullptr;
  /// A dictionary object of parameter values. Parameters with the same name
  /// as the technique's parameter override the technique's parameter value.
  std::map<std::string, material_value> values;
};
}

#endif
