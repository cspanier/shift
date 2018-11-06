#include "shift/rc/importer_gltf/accessor.hpp"
#include <shift/parser/json/json.hpp>

namespace shift::rc::gltf
{
accessor::accessor(const parser::json::object& json_object)
: child_of_root_property(json_object)
{
  using std::get;

  buffer_view_id = get<std::string>(json_object.at("bufferView"));

  auto offset_value = get<double>(json_object.at("byteOffset"));
  if (std::floor(offset_value) != offset_value)
  {
    BOOST_THROW_EXCEPTION(
      parse_error() << parse_error_info(
        "Illegal non-integer value '" + std::to_string(offset_value) +
        "' for property 'byteOffset' in 'bufferView' object."));
  }
  if (offset_value < 0)
  {
    BOOST_THROW_EXCEPTION(
      parse_error() << parse_error_info(
        "Illegal negative value '" + std::to_string(offset_value) +
        "' for property 'byteOffset' in 'bufferView' object."));
  }
  offset = static_cast<std::size_t>(offset_value);

  auto stride_iter = json_object.find("byteStride");
  if (stride_iter != json_object.end())
  {
    auto stride_value = get<double>(stride_iter->second);
    if (std::floor(stride_value) != stride_value)
    {
      BOOST_THROW_EXCEPTION(
        parse_error() << parse_error_info(
          "Illegal non-integer value '" + std::to_string(stride_value) +
          "' for property 'byteStride' in 'bufferView' object."));
    }
    if (stride_value < 0 || stride_value > 255)
    {
      BOOST_THROW_EXCEPTION(
        parse_error() << parse_error_info(
          "Illegal out of range value '" + std::to_string(stride_value) +
          "' for property 'byteStride' in 'bufferView' object."));
    }
    stride = static_cast<std::size_t>(stride_value);
  }

  auto component_type_number = get<double>(json_object.at("componentType"));
  if (component_type_number == 5120)
    component_type = accessor_component_type::int8;
  else if (component_type_number == 5121)
    component_type = accessor_component_type::uint8;
  else if (component_type_number == 5122)
    component_type = accessor_component_type::int16;
  else if (component_type_number == 5123)
    component_type = accessor_component_type::uint16;
  else if (component_type_number == 5126)
    component_type = accessor_component_type::float32;
  else
  {
    BOOST_THROW_EXCEPTION(
      parse_error() << parse_error_info(
        "Illegal value '" + std::to_string(component_type_number) +
        "' for property 'componentType' in 'bufferView' object."));
  }

  auto count_value = get<double>(json_object.at("count"));
  if (std::floor(count_value) != count_value)
  {
    BOOST_THROW_EXCEPTION(parse_error() << parse_error_info(
                            "Illegal non-integer value '" +
                            std::to_string(count_value) +
                            "' for property 'count' in 'bufferView' object."));
  }
  if (count_value < 0)
  {
    BOOST_THROW_EXCEPTION(parse_error() << parse_error_info(
                            "Illegal negative value '" +
                            std::to_string(count_value) +
                            "' for property 'count' in 'bufferView' object."));
  }
  count = static_cast<std::size_t>(count_value);

  const auto& type_name = get<std::string>(json_object.at("type"));
  if (type_name == "SCALAR")
    type = accessor_type::scalar;
  else if (type_name == "VEC2")
    type = accessor_type::vec2;
  else if (type_name == "VEC3")
    type = accessor_type::vec3;
  else if (type_name == "VEC4")
    type = accessor_type::vec4;
  else if (type_name == "MAT2")
    type = accessor_type::mat2;
  else if (type_name == "MAT3")
    type = accessor_type::mat3;
  else if (type_name == "MAT4")
    type = accessor_type::mat4;
  else
  {
    BOOST_THROW_EXCEPTION(parse_error() << parse_error_info(
                            "Illegal value '" + type_name +
                            "' for property 'type' in object 'bufferView'."));
  }

  /// ToDo: Read min and max.
  min = math::vector4<float>{0.0f, 0.0f, 0.0f, 0.0f};
  max = math::vector4<float>{0.0f, 0.0f, 0.0f, 0.0f};
}
}
