#ifndef SHIFT_SCENE_TYPES_HPP
#define SHIFT_SCENE_TYPES_HPP

#include <cstdint>
#include <shift/core/abstract_factory.hpp>
#include <shift/math/utility.hpp>

namespace shift
{
namespace scene
{
  using namespace math::literals;

  /// Each entity type needs to have a globally unique id used for object
  /// construction.
  using entity_type_guid = std::uint32_t;

  class entity_extension;
  class entity;
  class camera_entity;
  class model_entity;

  class renderer;
  class entity_system;

  using extension_factory_t =
    core::abstract_factory<entity_type_guid, entity_extension,
                           core::factory_using_unique_ptr, entity&>;
}
}

#endif
