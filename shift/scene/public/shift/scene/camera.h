#ifndef SHIFT_SCENE_CAMERA_H
#define SHIFT_SCENE_CAMERA_H

#include "shift/scene/entity.h"

namespace shift
{
namespace scene
{
  class camera_entity;

  ///
  class camera_extension : public entity_extension
  {
  public:
    ///
    camera_extension(scene::entity& entity) : entity_extension(entity)
    {
    }

  protected:
    ///
    camera_entity& camera()
    {
      return *reinterpret_cast<camera_entity*>(_entity);
    }
  };

  enum class camera_change_flag : change_flags
  {
    fov_y = scene::next_free_flag<entity_change_flag>(),

    next_free_flag
  };

  ///
  class camera_entity : public entity
  {
  public:
    static constexpr entity_type_guid type_guid = 0x0B797BE5;

  public:
    camera_entity();

    ///
    void update() override;

  protected:
    float fov_y = 70_fdeg;
    change_flags _change_flags;
  };
}
}

#endif
