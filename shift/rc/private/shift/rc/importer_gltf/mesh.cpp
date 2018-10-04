#include "shift/rc/importer_gltf/mesh.h"
#include <shift/parser/json/json.h>

namespace shift::rc::gltf
{
mesh::mesh(const parser::json::object& json_object)
: child_of_root_property(json_object)
{
  using std::get;

  auto primitives_iter = json_object.find("primitives");
  if (primitives_iter != json_object.end())
  {
    for (const auto& primitive_iter :
         get<parser::json::array>(primitives_iter->second))
    {
      primitives.push_back(std::make_unique<gltf::primitive>(
        get<parser::json::object>(primitive_iter)));
    }
  }
}
}
