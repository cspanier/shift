#ifndef SHIFT_RESOURCE_DB_MATERIAL_DESCRIPTOR_HPP
#define SHIFT_RESOURCE_DB_MATERIAL_DESCRIPTOR_HPP

#include "shift/resource_db/types.hpp"
#include "shift/resource_db/resource_ptr.hpp"

namespace shift::resource_db
{
template <>
struct resource_traits<material_descriptor>
{
  static constexpr resource_type type_id = resource_type::material_descriptor;
};

class material_descriptor final : public resource_base
{
public:
  /// Default constructor.
  material_descriptor() noexcept
  : resource_base(resource_traits<material_descriptor>::type_id)
  {
  }

  /// @see resource_base::load.
  void load(resource_id id, boost::iostreams::filtering_istream& stream) final;

  /// @see resource_base::save.
  void save(boost::iostreams::filtering_ostream& stream) const final;

public:
protected:
  /// @see resource_base::hash.
  void hash(crypto::sha256& context) const final;
};
}

#endif
