#include "shift/rc/importer_gltf/primitive.h"
#include <shift/parser/json/json.h>
#include <cmath>

namespace shift::rc::gltf
{
primitive::primitive(const parser::json::object& json_object)
: property(json_object)
{
  using std::get;

  auto attributes_iter = json_object.find("attributes");
  if (attributes_iter != json_object.end())
  {
    for (const auto& attribute_iter :
         get<parser::json::object>(attributes_iter->second))
    {
      attributes.insert(std::make_pair(
        attribute_iter.first,
        gltf::primitive_attribute(get<std::string>(attribute_iter.second))));
    }
  }

  auto indices_iter = json_object.find("indices");
  if (indices_iter != json_object.end())
  {
    indices_id = get<std::string>(indices_iter->second);
  }

  auto material_iter = json_object.find("material");
  if (material_iter != json_object.end())
  {
    material_id = get<std::string>(material_iter->second);
  }

  auto mode_iter = json_object.find("mode");
  if (mode_iter != json_object.end())
  {
    auto mode_value = get<double>(mode_iter->second);
    if (std::floor(mode_value) != mode_value)
    {
      BOOST_THROW_EXCEPTION(parse_error() << parse_error_info(
                              "Illegal non-integer value '" +
                              std::to_string(mode_value) +
                              "' for property 'mode' in 'primitive' object."));
    }
    switch (static_cast<std::size_t>(mode_value))
    {
    case 0:
      mode = primitive_mode::points;
      break;
    case 1:
      mode = primitive_mode::lines;
      break;
    case 2:
      mode = primitive_mode::line_loop;
      break;
    case 3:
      mode = primitive_mode::line_strip;
      break;
    case 4:
      mode = primitive_mode::triangles;
      break;
    case 5:
      mode = primitive_mode::triangle_strip;
      break;
    case 6:
      mode = primitive_mode::triangle_fan;
      break;
    default:
      BOOST_THROW_EXCEPTION(parse_error() << parse_error_info(
                              "Illegal value '" + std::to_string(mode_value) +
                              "' for property 'mode' in 'primitive' object."));
    }
  }
}
}
