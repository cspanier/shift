#include "shift/resource/mesh.hpp"
#include <shift/serialization2/all.hpp>
#include <shift/math/serialization2.hpp>
#include <shift/crypto/sha256.hpp>

namespace shift::resource
{
std::size_t vertex_attribute_size(
  vertex_attribute_component_type component_type,
  vertex_attribute_data_type data_type)
{
  std::size_t size;
  switch (component_type)
  {
  case vertex_attribute_component_type::int8:
  case vertex_attribute_component_type::uint8:
    size = 1;
    break;

  case vertex_attribute_component_type::int16:
  case vertex_attribute_component_type::uint16:
    size = 2;
    break;

  case vertex_attribute_component_type::int32:
  case vertex_attribute_component_type::uint32:
  case vertex_attribute_component_type::float32:
    size = 4;
    break;

  default:
    BOOST_ASSERT(false);
    size = 0;
  }

  switch (data_type)
  {
  case vertex_attribute_data_type::scalar:
    return size;

  case vertex_attribute_data_type::vec2:
    return 2 * size;

  case vertex_attribute_data_type::vec3:
    return 3 * size;

  case vertex_attribute_data_type::vec4:
  case vertex_attribute_data_type::mat22:
    return 4 * size;

  case vertex_attribute_data_type::mat33:
    return 9 * size;

  case vertex_attribute_data_type::mat44:
    return 16 * size;

  default:
    BOOST_ASSERT(false);
    return 0;
  }
}

crypto::sha256& operator<<(crypto::sha256& context,
                           const vertex_attribute& value)
{
  value.vertex_buffer_view.hash(context);
  context << value.offset << value.size << value.stride << value.usage
          << value.component_type << value.data_type;
  return context;
}

serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, vertex_attribute& value)
{
  archive >> value.vertex_buffer_view >> value.offset >> value.size >>
    value.stride >> value.usage >> value.component_type >> value.data_type;
  return archive;
}

serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive,
  const vertex_attribute& value)
{
  archive << value.vertex_buffer_view << value.offset << value.size
          << value.stride << value.usage << value.component_type
          << value.data_type;
  return archive;
}

crypto::sha256& operator<<(crypto::sha256& context, const sub_mesh& value)
{
  context << value.topology << value.vertex_offset << value.first_index
          << value.index_count;
  return context;
}

serialization2::compact_input_archive<>& operator>>(
  serialization2::compact_input_archive<>& archive, sub_mesh& value)
{
  archive >> value.topology >> value.vertex_offset >> value.first_index >>
    value.index_count;
  return archive;
}

serialization2::compact_output_archive<>& operator<<(
  serialization2::compact_output_archive<>& archive, const sub_mesh& value)
{
  archive << value.topology << value.vertex_offset << value.first_index
          << value.index_count;
  return archive;
}

void mesh::load(resource_id id, boost::iostreams::filtering_istream& stream)
{
  _id = id;
  serialization2::compact_input_archive<> archive{stream};
  archive >> vertex_attributes >> index_buffer_view >> index_data_type >>
    sub_meshes >> bounding_box;
}

void mesh::save(boost::iostreams::filtering_ostream& stream) const
{
  serialization2::compact_output_archive<> archive{stream};
  archive << vertex_attributes << index_buffer_view << index_data_type
          << sub_meshes << bounding_box;
}

void mesh::hash(crypto::sha256& context) const
{
  for (const auto& attribute : vertex_attributes)
    context << attribute;
  index_buffer_view.hash(context);
  context << index_data_type;
  for (const auto& sub_mesh : sub_meshes)
    context << sub_mesh;
}
}
