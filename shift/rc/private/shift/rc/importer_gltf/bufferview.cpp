#include "shift/rc/importer_gltf/bufferview.hpp"
#include <shift/parser/json/json.hpp>

namespace shift::rc::gltf
{
buffer_view::buffer_view(const parser::json::object& json_object)
: child_of_root_property(json_object)
{
  using std::get;

  buffer_id = get<std::string>(json_object.at("buffer"));
  offset = static_cast<std::size_t>(get<double>(json_object.at("byteOffset")));

  auto length_iter = json_object.find("byteLength");
  if (length_iter != json_object.end())
  {
    length = static_cast<std::size_t>(get<double>(length_iter->second));
  }

  // auto target_iter = json_object.find("targetNumber");
  // if (target_iter != json_object.end())
  //{
  //  auto target_number =
  //    static_cast<std::int32_t>(get<double>(target_iter->second));
  //  switch (target_number)
  //  {
  //  case 34962:
  //    target = buffer_view_target::array_buffer;
  //    break;
  //  case 34963:
  //    target = buffer_view_target::element_array_buffer;
  //    break;
  //  default:
  //    return;  /// ToDo: Throw exception.
  //  }
  //}
}
}
