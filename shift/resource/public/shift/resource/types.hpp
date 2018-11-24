#ifndef SHIFT_RESOURCE_TYPES_HPP
#define SHIFT_RESOURCE_TYPES_HPP

namespace shift::resource
{
class buffer;
class buffer_view;
class font;
class image;
class material;
class material_descriptor;
class mesh;
class model;
class scene;
struct scene_node;
class shader;
class resource_group;

template <typename T>
struct resource_traits;
}

#endif
