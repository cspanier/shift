#include "shift/rc/importer_gltf/material.h"
#include <shift/parser/json/json.h>

namespace shift::rc::gltf
{
material::material(const parser::json::object& json_object)
: child_of_root_property(json_object)
{
}
}
