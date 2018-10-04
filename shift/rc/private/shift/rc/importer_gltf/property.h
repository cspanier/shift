#ifndef SHIFT_RC_IMPORTER_GLTF_PROPERTY_H
#define SHIFT_RC_IMPORTER_GLTF_PROPERTY_H

#include "shift/rc/importer_gltf/types.h"

namespace shift::rc::gltf
{
/// glTF property.
struct property
{
  /// Constructor.
  property(const parser::json::object& json_object);

  //"extensions" : {
  //    "$ref" : "extension.schema.json"
  //},
  //"extras" : {
  //    "$ref" : "extras.schema.json"
  //}
};
}

#endif
