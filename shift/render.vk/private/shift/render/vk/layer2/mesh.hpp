#ifndef SHIFT_RENDER_VK_LAYER2_MESH_HPP
#define SHIFT_RENDER_VK_LAYER2_MESH_HPP

#include <memory>
#include <array>
#include <shift/resource/mesh.hpp>
#include "shift/render/vk/types.hpp"
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/smart_ptr.hpp"
#include "shift/render/vk/mesh.hpp"
#include "shift/render/vk/layer2/object.hpp"
#include "shift/render/vk/layer2/buffer.hpp"

namespace shift::resource
{
class mesh;
}

namespace shift::render::vk::layer2
{
///
class mesh final : public vk::mesh, public child_object, public parent_object
{
public:
  enum class vertex_attribute_usage : std::uint8_t
  {
    position,
    normal,
    tangent,
    bitangent,
    texcoord
  };
  static constexpr std::size_t vertex_attribute_usage_count =
    static_cast<std::size_t>(vertex_attribute_usage::texcoord) + 1;

  struct vertex_attribute_t
  {
    boost::intrusive_ptr<vk::layer2::buffer> buffer;

    /// The offset of the buffer view in bytes.
    std::uint32_t buffer_offset = 0;

    /// The length of the buffer view in bytes.
    std::uint32_t buffer_size = 0;

    /// The offset relative to the start of the vertex in bytes. This must be
    /// a multiple of the size of the data type.
    std::uint16_t attribute_offset = 0;

    /// The stride, in bytes, between two neighboring attributes referenced by
    /// this accessor. When this value is either zero or the size of the
    /// attribute's type, attributes are tightly packed.
    std::uint8_t attribute_stride = 4;

    /// The attribute's data type.
    vk::format format = vk::format::undefined;

    /// The size of the vertex attribute. This can also be computed using
    /// component_type and data_type, but it is provided for convenience.
    std::uint8_t attribute_size = 4;
  };

  struct index_attribute_t
  {
    boost::intrusive_ptr<vk::layer2::buffer> buffer;

    /// The offset of the buffer view in bytes.
    std::uint32_t buffer_offset = 0;

    /// The length of the buffer view in bytes.
    std::uint32_t buffer_size = 0;

    /// Index element type.
    vk::index_type index_type = vk::index_type::uint32;
  };

  using vertex_attributes_t =
    std::array<vertex_attribute_t, vertex_attribute_usage_count>;

public:
  ///
  mesh(vk::layer1::device& device, vertex_attributes_t vertex_attributes,
       index_attribute_t index_attribute);

  mesh(const mesh&) = delete;
  mesh(mesh&&) = delete;
  ~mesh() noexcept final;
  mesh& operator=(const mesh&) = delete;
  mesh& operator=(mesh&&) = delete;

  /// @see parent_object::signal.
  void signal(availability_state signal) final;

private:
  vk::layer1::device* _device = nullptr;
  vertex_attributes_t _vertex_attributes;
  index_attribute_t _index_attribute;
};
}

#endif
