#ifndef SHIFT_RC_IMPORTER_GLTF_GLTF_H
#define SHIFT_RC_IMPORTER_GLTF_GLTF_H

#include <cstdint>
#include <map>
#include <string>
#include <memory>
#include "shift/rc/importer_gltf/types.h"
#include "shift/rc/importer_gltf/property.h"
#include "shift/rc/importer_gltf/accessor.h"
#include "shift/rc/importer_gltf/buffer.h"
#include "shift/rc/importer_gltf/bufferview.h"
#include "shift/rc/importer_gltf/material.h"
#include "shift/rc/importer_gltf/mesh.h"

namespace shift::rc::gltf
{
/// The root object for a glTF asset.
struct gltf_root : public property
{
  /// Constructor.
  /// @throw parse_error
  ///   This exception is thrown if anything goes wrong.
  gltf_root(const parser::json::object& json_object);

  std::map<std::string, std::unique_ptr<accessor>> accessors;
  std::map<std::string, std::unique_ptr<buffer_view>> buffer_views;
  std::map<std::string, std::unique_ptr<buffer>> buffers;
  std::map<std::string, std::unique_ptr<material>> materials;
  std::map<std::string, std::unique_ptr<mesh>> meshes;
};
}

#endif
