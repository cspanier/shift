#ifndef SHIFT_RC_IMPORTER_GLTF_BUFFER_HPP
#define SHIFT_RC_IMPORTER_GLTF_BUFFER_HPP

#include <cstdint>
#include <string>
#include "shift/rc/importer_gltf/types.hpp"
#include "shift/rc/importer_gltf/childofrootproperty.hpp"

namespace shift::rc::gltf
{
enum class buffer_type
{
  array_buffer,
  text
};

/// A buffer points to binary geometry, animation, or skins.
struct buffer : public child_of_root_property
{
  /// Constructor.
  buffer(const parser::json::object& json_object);

  /// The uri of the buffer. Relative paths are relative to the .gltf file.
  /// Instead of referencing an external file, the uri can also be a
  /// data-uri.
  /// "format" : "uri"
  /// "required" : true
  std::string uri;
  /// The length of the buffer in bytes.
  std::size_t byte_length = 0;
  /// "type" : "string"
  /// "enum" : ["arraybuffer", "text"]
  /// "default" : "arraybuffer"
  buffer_type type;
};
}

#endif
