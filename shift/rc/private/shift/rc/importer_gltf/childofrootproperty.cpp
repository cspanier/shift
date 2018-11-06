#include "shift/rc/importer_gltf/childofrootproperty.hpp"
#include <shift/parser/json/json.hpp>

namespace shift::rc::gltf
{
child_of_root_property::child_of_root_property(
  const parser::json::object& json_object)
: property(json_object)
{
}
}
