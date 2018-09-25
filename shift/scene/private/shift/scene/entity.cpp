#include "shift/scene/entity.h"
#include <algorithm>

namespace shift
{
namespace scene
{
  entity_extension::~entity_extension() = default;

  entity::entity() noexcept = default;

  entity::~entity()
  {
    // Assume this entity has been properly detatched from the scene graph.
    BOOST_ASSERT(!_parent);
  }

  void entity::update()
  {
    _change_flags = 0;
  }

  bool entity::attach(entity& child)
  {
    if (child.parent())
      return false;
    _children.push_back(&child);
    child.parent(this);
    return true;
  }

  bool entity::detach(entity& child)
  {
    if (child.parent() != this)
      return false;

    _children.erase(
      std::remove(std::begin(_children), std::end(_children), &child),
      std::end(_children));
    child.parent(nullptr);
    return true;
  }

  const std::vector<entity*>& entity::children() const
  {
    return _children;
  }

  entity* entity::parent() const noexcept
  {
    return _parent;
  }

  math::matrix44<float> entity::to_parent() const
  {
    return math::make_rotation_matrix<4, 4, float>(_rotation) *
           math::make_translation_matrix<4, 4, float>(_position);
  }

  entity_extension* entity::graphics() const
  {
    return _graphics.get();
  }

  entity_extension* entity::physics() const
  {
    return _physics.get();
  }

  entity_extension* entity::ai() const
  {
    return _ai.get();
  }

  void entity::parent(entity* parent_entity) noexcept
  {
    _parent = parent_entity;
  }

  void entity::graphics(std::unique_ptr<entity_extension> extension)
  {
    _graphics = std::move(extension);
  }

  void entity::physics(std::unique_ptr<entity_extension> extension)
  {
    _physics = std::move(extension);
  }

  void entity::ai(std::unique_ptr<entity_extension> extension)
  {
    _ai = std::move(extension);
  }
}
}
