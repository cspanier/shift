#include "shift/rc/importer_gltf/childofrootproperty.h"
#include <shift/parser/json/json.h>

namespace shift::rc::gltf
{
child_of_root_property::child_of_root_property(
  const parser::json::object& json_object)
: property(json_object)
{
}
}
