#include "shift/rc/importer_gltf/material.hpp"
#include <shift/parser/json/json.hpp>

namespace shift::rc::gltf
{
material::material(const parser::json::object& json_object)
: child_of_root_property(json_object)
{
}
}
