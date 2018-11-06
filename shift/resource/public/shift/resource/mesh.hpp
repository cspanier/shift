#ifndef SHIFT_RESOURCE_MESH_HPP
#define SHIFT_RESOURCE_MESH_HPP

#include <limits>
#include <vector>
#include <shift/math/vector.hpp>
#include <shift/math/aabb.hpp>
#include "shift/resource/resource_ptr.hpp"
#include "shift/resource/buffer_view.hpp"

namespace shift::resource
{
///
enum class vertex_attribute_usage : std::uint8_t
{
  position,
  normal,
  tangent,
  bitangent,
  texcoord
};

///
enum class vertex_attribute_component_type : std::uint8_t
{
  int8,
  uint8,
  int16,
  uint16,
  int32,
  uint32,
  float32
};

///
enum class vertex_attribute_data_type : std::uint8_t
{
  scalar,
  vec2,
  vec3,
  vec4,
  mat22,
  mat33,
  mat44
};

///
enum class vertex_index_data_type : std::uint8_t
{
  uint8,
  uint16,
  uint32
};

std::size_t vertex_attribute_size(
  vertex_attribute_component_type component_type,
  vertex_attribute_data_type data_type);

///
struct vertex_attribute
{
  /// The buffer_view to use.
  buffer_view vertex_buffer_view;

  /// The offset relative to the start of the vertex in bytes. This must be a
  /// multiple of the size of the data type.
  std::uint16_t offset;

  /// The stride, in bytes, between two neighboring attributes referenced by
  /// this accessor. When this value is either zero or the size of the
  /// attribute's type, attributes are tightly packed.
  std::uint8_t stride;

  /// Semantic description of what kind of data is referenced by this
  /// accessor.
  vertex_attribute_usage usage;

  /// The datatype of components in this vertex attribute.
  vertex_attribute_component_type component_type;

  /// Specifies if the attribute is a scalar, vector, or matrix, and the
  /// number of elements in the vector or matrix.
  vertex_attribute_data_type data_type;

  /// The size of the vertex attribute. This can also be computed using
  /// component_type and data_type, but it is provided for convenience.
  std::uint8_t size;

  ///
  friend crypto::sha256& operator<<(crypto::sha256& context,
                                    const vertex_attribute& value);

  ///
  friend serialization2::compact_input_archive<>& operator>>(
    serialization2::compact_input_archive<>& archive, vertex_attribute& value);

  ///
  friend serialization2::compact_output_archive<>& operator<<(
    serialization2::compact_output_archive<>& archive,
    const vertex_attribute& value);
};

///
enum class primitive_topology : std::uint8_t
{
  points,
  line_list,
  line_loop,
  line_strip,
  triangle_list,
  triangle_strip,
  triangle_fan
};

///
struct sub_mesh
{
  primitive_topology topology;
  std::uint32_t vertex_offset;
  std::uint32_t first_index;
  std::uint32_t index_count;

  ///
  friend crypto::sha256& operator<<(crypto::sha256& context,
                                    const sub_mesh& value);

  ///
  friend serialization2::compact_input_archive<>& operator>>(
    serialization2::compact_input_archive<>& archive, sub_mesh& value);

  ///
  friend serialization2::compact_output_archive<>& operator<<(
    serialization2::compact_output_archive<>& archive, const sub_mesh& value);
};

///
class mesh final : public resource_base
{
public:
  static constexpr resource_type static_type = resource_type::mesh;

  /// Default constructor.
  mesh() noexcept : resource_base(static_type)
  {
  }

  /// @see resource_base::load.
  void load(resource_id id, boost::iostreams::filtering_istream& stream) final;

  /// @see resource_base::save.
  void save(boost::iostreams::filtering_ostream& stream) const final;

public:
  /// The list of vertex attribute definitions shared by each sub_mesh.
  std::vector<vertex_attribute> vertex_attributes;

  /// The buffer_view to use.
  resource::buffer_view index_buffer_view;

  /// Type of each single index value.
  vertex_index_data_type index_data_type = vertex_index_data_type::uint32;

  /// List of separate draw calls all sharing the same vertex layout, index
  /// buffer, and material.
  std::vector<sub_mesh> sub_meshes;

  math::aabb<3, float> bounding_box;

protected:
  /// @see resource_base::hash.
  void hash(crypto::sha256& context) const final;
};
}

#endif
