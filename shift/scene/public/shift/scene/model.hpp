#ifndef SHIFT_SCENE_MODEL_HPP
#define SHIFT_SCENE_MODEL_HPP

#include "shift/scene/entity.hpp"

namespace shift
{
namespace scene
{
  class model_entity;

  enum class model_change_flag
  {
    fov_y = 0b00000001
  };

  using model_change_flags = core::bit_field<model_change_flag>;

  ///
  class model_extension : public entity_extension
  {
  public:
    ///
    model_extension(scene::entity& entity) : entity_extension(entity)
    {
    }

  protected:
    ///
    model_entity& model()
    {
      return *reinterpret_cast<model_entity*>(_entity);
    }
  };

  ///
  class model_entity : public entity
  {
  public:
    static constexpr entity_type_guid type_guid = 0x9C76B8FE;

  public:
    ///
    model_entity() = default;

    ///
    void update() override;
  };
}
}

#endif
