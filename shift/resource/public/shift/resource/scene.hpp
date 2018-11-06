#ifndef SHIFT_RESOURCE_SCENE_HPP
#define SHIFT_RESOURCE_SCENE_HPP

#include <memory>
#include <vector>
#include <shift/math/matrix.hpp>
#include <shift/serialization2/pointer.hpp>
#include "shift/resource/resource_ptr.hpp"
#include "shift/resource/material.hpp"
#include "shift/resource/mesh.hpp"

namespace shift::resource
{
struct scene_node
{
  std::vector<scene_node*> children;

  math::matrix44<float> transform = math::make_identity_matrix<4, 4, float>();
  resource_ptr<resource::mesh> mesh;
  resource_ptr<resource::material> material;
};

///
class scene final : public resource_base
{
public:
  static constexpr resource_type static_type = resource_type::scene;

  /// Default constructor.
  scene() noexcept : resource_base(static_type)
  {
  }

  /// @see resource_base::load.
  void load(resource_id id, boost::iostreams::filtering_istream& stream) final;

  /// @see resource_base::save.
  void save(boost::iostreams::filtering_ostream& stream) const final;

public:
  std::vector<std::unique_ptr<scene_node>> nodes;
  scene_node* root = nullptr;

protected:
  /// @see resource_base::hash.
  void hash(crypto::sha256& context) const final;
};
}

#endif
