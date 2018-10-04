#ifndef SHIFT_RC_IMPORTER_GLTF_TYPES_H
#define SHIFT_RC_IMPORTER_GLTF_TYPES_H

#include <shift/core/exception.h>
#include <shift/parser/json/json.h>

namespace shift::rc::gltf
{
struct accessor;
struct buffer;
struct buffer_view;
struct child_of_root_property;
struct gltf_root;
struct material;
struct material;
struct mesh;
struct primitive;
struct primitive_attribute;
struct property;
struct technique;

/// Primary exception type used throughout this library. Usually this
/// exception has a parse_error_info attached containing more
/// information about the error.
struct parse_error : virtual core::runtime_error
{
};

using parse_error_info =
  boost::error_info<struct parse_error_info_tag, std::string>;
}

#endif
