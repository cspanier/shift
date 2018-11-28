#ifndef SHIFT_RESOURCE_DB_SCENE_HPP
#define SHIFT_RESOURCE_DB_SCENE_HPP

#include <memory>
#include <vector>
#include <shift/math/matrix.hpp>
#include <shift/serialization2/pointer.hpp>
#include "shift/resource_db/types.hpp"
#include "shift/resource_db/resource_ptr.hpp"
#include "shift/resource_db/material.hpp"
#include "shift/resource_db/mesh.hpp"

namespace shift::resource_db
{
template <>
struct resource_traits<scene>
{
  static constexpr resource_type type_id = resource_type::scene;
};

struct scene_node
{
  std::vector<scene_node*> children;

  math::matrix44<float> transform = math::make_identity_matrix<4, 4, float>();
  resource_ptr<resource_db::mesh> mesh;
  resource_ptr<resource_db::material> material;
};

///
class scene final : public resource_base
{
public:
  /// Default constructor.
  scene() noexcept : resource_base(resource_traits<scene>::type_id)
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
