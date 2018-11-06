#ifndef SHIFT_RC_IMPORTER_GLTF_GLTF_HPP
#define SHIFT_RC_IMPORTER_GLTF_GLTF_HPP

#include <cstdint>
#include <map>
#include <string>
#include <memory>
#include "shift/rc/importer_gltf/types.hpp"
#include "shift/rc/importer_gltf/property.hpp"
#include "shift/rc/importer_gltf/accessor.hpp"
#include "shift/rc/importer_gltf/buffer.hpp"
#include "shift/rc/importer_gltf/bufferview.hpp"
#include "shift/rc/importer_gltf/material.hpp"
#include "shift/rc/importer_gltf/mesh.hpp"

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
