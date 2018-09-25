#ifndef SHIFT_RESOURCE_MATERIAL_DESCRIPTOR_H
#define SHIFT_RESOURCE_MATERIAL_DESCRIPTOR_H

#include "shift/resource/resource_ptr.h"

namespace shift::resource
{
class material_descriptor final : public resource_base
{
public:
  static constexpr resource_type static_type = resource_type::material;

  /// Default constructor.
  material_descriptor() noexcept : resource_base(static_type)
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
