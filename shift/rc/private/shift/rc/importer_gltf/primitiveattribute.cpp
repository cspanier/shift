#include <utility>

#include "shift/rc/importer_gltf/primitiveattribute.hpp"

namespace shift::rc::gltf
{
primitive_attribute::primitive_attribute(std::string accessor_id)
: accessor_id(std::move(accessor_id))
{
}
}
