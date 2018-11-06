#ifndef SHIFT_RC_IMPORTER_GLTF_ACCESSOR_HPP
#define SHIFT_RC_IMPORTER_GLTF_ACCESSOR_HPP

#include <cstdint>
#include <string>
#include <shift/math/vector.hpp>
#include "shift/rc/importer_gltf/types.hpp"
#include "shift/rc/importer_gltf/childofrootproperty.hpp"

namespace shift::rc::gltf
{
struct buffer_view;

enum class accessor_component_type
{
  int8 = 5120,
  uint8 = 5121,
  int16 = 5122,
  uint16 = 5123,
  float32 = 5126
};

enum class accessor_type
{
  scalar,
  vec2,
  vec3,
  vec4,
  mat2,
  mat3,
  mat4
};

/// A typed view into a buffer_view. A buffer_view contains raw binary data.
/// An accessor provides a typed view into a buffer_view or a subset of a
/// buffer_view similar to how WebGL's `vertexAttribPointer()` defines an
/// attribute in a buffer.
struct accessor : public child_of_root_property
{
  /// Constructor.
  accessor(const parser::json::object& json_object);

  /// The ID of the buffer_view.
  std::string buffer_view_id;
  /// The buffer_view referenced by buffer_view_id.
  gltf::buffer_view* buffer_view = nullptr;
  /// The offset relative to the start of the buffer_view in bytes. This
  /// must be a multiple of the size of the data type.
  std::size_t offset;
  /// The stride, in bytes, between attributes referenced by this accessor.
  /// When this is zero, the attributes are tightly packed.
  std::size_t stride = 0;
  /// The datatype of components in the attribute. All valid values
  /// correspond to WebGL enums. The corresponding typed arrays are
  /// `Int8Array`, `Uint8Array`, `Int16Array`, `Uint16Array`, and
  /// `Float32Array`, respectively.
  accessor_component_type component_type;
  /// The number of attributes referenced by this accessor, not to be
  /// confused with the number of bytes or number of components.
  std::size_t count;
  /// Specifies if the attribute is a scalar, vector, or matrix, and the
  /// number of elements in the vector or matrix.
  accessor_type type;
  /// Minimum value of each component in this attribute. When both min and
  /// max arrays are defined, they have the same length. The length is
  /// determined by the value of the type property.
  math::vector4<float> min;
  /// Maximum value of each component in this attribute. When both min and
  /// max arrays are defined, they have the same length. The length is
  /// determined by the value of the type property.
  math::vector4<float> max;
};
}

#endif
