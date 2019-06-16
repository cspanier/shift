#ifndef SHIFT_SCENE_ENTITY_HPP
#define SHIFT_SCENE_ENTITY_HPP

#include <memory>
#include <vector>
#include <shift/core/types.hpp>
#include <shift/core/bit_field.hpp>
#include <shift/math/vector.hpp>
#include <shift/math/matrix.hpp>
#include <shift/math/quaternion.hpp>
#include "shift/scene/types.hpp"

namespace shift
{
namespace scene
{
  class entity;

  using change_flags = std::uint32_t;

  /// Common base class for enitity extensions.
  class entity_extension
  {
  public:
    ///
    entity_extension(scene::entity& entity) : _entity(&entity)
    {
    }

    ///
    virtual ~entity_extension();

  protected:
    scene::entity* _entity = nullptr;
  };

  ///
  enum class entity_change_flag : change_flags
  {
    position,
    rotation,

    next_free_flag
  };

  ///
  template <typename TEnum>
  constexpr change_flags next_free_flag()
  {
    return static_cast<change_flags>(TEnum::next_free_flag);
  }

  /// An entity is the base class for all scene objects. It mainly serves as an
  /// extendible abstraction wrapper for specific object extensions (renderer,
  /// ai, physics, ...).
  class entity
  {
  public:
    ///
    entity() noexcept;

    entity(const entity&) = delete;
    entity(entity&&) = default;

    ///
    virtual ~entity();

    entity& operator=(const entity&) = delete;
    entity& operator=(entity&&) = default;

    /// The per-entity update may only modify internal state.
    virtual void update();

    ///
    bool attach(entity& child);

    ///
    bool detach(entity& child);

    ///
    const std::vector<entity*>& children() const;

    ///
    entity* parent() const noexcept;

    ///
    math::matrix44<float> to_parent() const;

    ///
    entity_extension* graphics() const;

    ///
    entity_extension* physics() const;

    ///
    entity_extension* ai() const;

    ///
    bool changed() const
    {
      return _change_flags != 0;
    }

    ///
    template <typename TEnum>
    bool changed(TEnum bit) const
    {
      auto mask = 1 << core::underlying_type_cast(bit);
      return (_change_flags & mask) != 0;
    }

  protected:
    ///
    void parent(entity* parent_entity) noexcept;

    ///
    template <typename TEnum>
    void change(TEnum bit)
    {
      auto mask = 1 << core::underlying_type_cast(bit);
      _change_flags |= mask;
    }

    entity* _parent = nullptr;
    std::vector<entity*> _children;
    std::unique_ptr<entity_extension> _graphics;
    std::unique_ptr<entity_extension> _physics;
    std::unique_ptr<entity_extension> _ai;

    math::vector3<float> _position = math::make_default_vector<3, float>(0.0f);
    math::quaternion<float> _rotation = math::make_identity_quaternion<float>();

  private:
    friend class entity_system;

    ///
    void graphics(std::unique_ptr<entity_extension> extension);

    ///
    void physics(std::unique_ptr<entity_extension> extension);

    ///
    void ai(std::unique_ptr<entity_extension> extension);

  private:
    change_flags _change_flags = 0;
  };
}
}

#endif
