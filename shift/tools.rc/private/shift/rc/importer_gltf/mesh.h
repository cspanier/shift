#ifndef SHIFT_RC_IMPORTER_GLTF_MESH_H
#define SHIFT_RC_IMPORTER_GLTF_MESH_H

#include <memory>
#include <vector>
#include "shift/rc/importer_gltf/types.h"
#include "shift/rc/importer_gltf/primitive.h"
#include "shift/rc/importer_gltf/childofrootproperty.h"

namespace shift::rc::gltf
{
/// A set of primitives to be rendered. A node can contain one or more
/// meshes. A node's transform places the mesh in the scene.
struct mesh : public child_of_root_property
{
  /// Constructor.
  mesh(const parser::json::object& json_object);

  /// An array of primitives, each defining geometry to be rendered with a
  /// material.
  std::vector<std::unique_ptr<primitive>> primitives;
};
}

#endif
