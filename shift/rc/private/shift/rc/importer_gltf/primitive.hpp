#ifndef SHIFT_RC_IMPORTER_GLTF_PRIMITIVE_HPP
#define SHIFT_RC_IMPORTER_GLTF_PRIMITIVE_HPP

#include <cstdint>
#include <map>
#include <string>
#include "shift/rc/importer_gltf/types.hpp"
#include "shift/rc/importer_gltf/property.hpp"
#include "shift/rc/importer_gltf/primitiveattribute.hpp"

namespace shift::rc::gltf
{
enum class primitive_mode
{
  points,
  lines,
  line_loop,
  line_strip,
  triangles,
  triangle_strip,
  triangle_fan
};

/// Geometry to be rendered with the given material.
struct primitive : public property
{
  /// Constructor.
  primitive(const parser::json::object& json_object);

  /// A map of accessors containing an attribute.
  std::map<std::string, primitive_attribute> attributes;
  /// The ID of the accessor that contains the indices.
  std::string indices_id;
  /// The accessor referenced by indices_id. When this is not defined, the
  /// primitives should be rendered without indices using `drawArrays()`.
  gltf::accessor* indices = nullptr;
  /// The ID of the material to apply to this primitive when rendering.
  std::string material_id;
  /// The material referenced by material_id.
  gltf::material* material = nullptr;
  /// The type of primitives to render. All valid values correspond to WebGL
  /// enums.
  primitive_mode mode = primitive_mode::triangles;
};
}

#endif
