#ifndef SHIFT_SCENE_ENTITYSYSTEM_H
#define SHIFT_SCENE_ENTITYSYSTEM_H

#include "shift/scene/entity.h"

namespace shift
{
namespace scene
{
  struct view
  {
    math::vector2<std::int32_t> position;
    math::vector2<std::uint32_t> size;
    camera_entity* camera;
  };

  class entity_system
  {
  public:
    /// Default constructor.
    entity_system();

    /// Assigns a new renderer instance. Changing the renderer effectively
    /// removes all render extensions on all scene nodes and creates new ones.
    void renderer(scene::renderer* new_renderer);

    ///
    std::unique_ptr<entity> create_instance(entity_type_guid type_guid);

    ///
    template <typename TEntity>
    std::unique_ptr<TEntity> create_instance()
    {
      return std::unique_ptr<TEntity>(
        static_cast<TEntity*>(create_instance(TEntity::type_guid).release()));
    }

    ///
    extension_factory_t& graphics_factory();

    ///
    extension_factory_t& physics_factory();

    ///
    extension_factory_t& ai_factory();

  private:
    using entity_factory_t =
      core::abstract_factory<entity_type_guid, entity,
                             core::factory_using_unique_ptr>;
    entity_factory_t _entity_factory;
    extension_factory_t _graphics_factory;
    extension_factory_t _physics_factory;
    extension_factory_t _ai_factory;

    scene::renderer* _renderer = nullptr;
  };
}
}

#endif
