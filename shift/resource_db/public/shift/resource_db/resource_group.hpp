#ifndef SHIFT_RESOURCE_DB_RESOURCE_GROUP_HPP
#define SHIFT_RESOURCE_DB_RESOURCE_GROUP_HPP

#include <cstdint>
#include <vector>
#include "shift/resource_db/resource_ptr.hpp"

namespace shift::resource_db
{
class image;
class mesh;

template <>
struct resource_traits<resource_group>
{
  static constexpr resource_type type_id = resource_type::resource_group;
};

///
class resource_group final : public resource_base
{
public:
  /// Default constructor.
  resource_group() noexcept
  : resource_base(resource_traits<resource_group>::type_id)
  {
  }

  /// @see resource_base::load.
  void load(resource_id id, boost::iostreams::filtering_istream& stream) final;

  /// @see resource_base::save.
  void save(boost::iostreams::filtering_ostream& stream) const final;

public:
  std::vector<resource_ptr<image>> images;
  std::vector<resource_ptr<mesh>> meshes;

protected:
  /// @see resource_base::hash.
  void hash(crypto::sha256& context) const final;
};
}

#endif
