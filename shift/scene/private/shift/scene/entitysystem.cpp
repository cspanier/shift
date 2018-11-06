#include "shift/scene/entitysystem.hpp"
#include "shift/scene/camera.hpp"
#include "shift/scene/model.hpp"

namespace shift
{
namespace scene
{
  entity_system::entity_system()
  {
    _entity_factory.template register_type<camera_entity>(
      camera_entity::type_guid);
    _entity_factory.template register_type<model_entity>(
      model_entity::type_guid);
  }

  void entity_system::renderer(scene::renderer* new_renderer)
  {
    if (_renderer)
    {
      /// ToDo: Delete all scene render extensions.
      _renderer = nullptr;
    }
    if (new_renderer)
    {
      _renderer = new_renderer;
      /// ToDo: Create new scene render extensions.
    }
  }

  std::unique_ptr<entity> entity_system::create_instance(
    entity_type_guid type_guid)
  {
    auto entity = _entity_factory.create_instance(type_guid);
    entity->graphics(_graphics_factory.create_instance(type_guid, *entity));
    entity->physics(_physics_factory.create_instance(type_guid, *entity));
    entity->ai(_ai_factory.create_instance(type_guid, *entity));

    return entity;
  }

  extension_factory_t& entity_system::graphics_factory()
  {
    return _graphics_factory;
  }

  extension_factory_t& entity_system::physics_factory()
  {
    return _physics_factory;
  }

  extension_factory_t& entity_system::ai_factory()
  {
    return _ai_factory;
  }
}
}
