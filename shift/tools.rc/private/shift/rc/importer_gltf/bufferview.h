#ifndef SHIFT_RC_IMPORTER_GLTF_BUFFERVIEW_H
#define SHIFT_RC_IMPORTER_GLTF_BUFFERVIEW_H

#include <cstdint>
#include <string>
#include "shift/rc/importer_gltf/types.h"
#include "shift/rc/importer_gltf/childofrootproperty.h"

namespace shift::rc::gltf
{
enum class buffer_view_target
{
  none = 0,
  array_buffer = 34962,
  element_array_buffer = 34963
};

/// A view into a buffer generally representing a subset of the buffer.
struct buffer_view : public child_of_root_property
{
  /// Constructor.
  buffer_view(const parser::json::object& json_object);

  /// The ID of the buffer.
  std::string buffer_id;
  /// The buffer referenced by buffer_id.
  gltf::buffer* buffer = nullptr;
  /// The offset into the buffer in bytes.
  std::size_t offset;
  /// The length of the bufferView in bytes.
  std::size_t length = 0;
  /// The target that the WebGL buffer should be bound to. All valid values
  /// correspond to WebGL enums. When this is not provided, the bufferView
  /// contains animation or skin data.
  buffer_view_target target = buffer_view_target::none;
};
}

#endif
