#include "shift/rc/importer_gltf/buffer.h"
#include <shift/parser/json/json.h>

namespace shift::rc::gltf
{
buffer::buffer(const parser::json::object& json_object)
: child_of_root_property(json_object)
{
  using parser::json::get;

  uri = get<std::string>(json_object.at("uri"));

  auto length_iter = json_object.find("byteLength");
  if (length_iter != json_object.end())
  {
    byte_length = static_cast<std::size_t>(get<double>(length_iter->second));
  }

  auto type_iter = json_object.find("type");
  if (type_iter != json_object.end())
  {
    const auto& type_name = get<std::string>(type_iter->second);
    if (type_name == "arraybuffer")
      type = buffer_type::array_buffer;
    else if (type_name == "text")
      type = buffer_type::text;
    else
      return;  /// ToDo: Throw exception.
  }
}
}
