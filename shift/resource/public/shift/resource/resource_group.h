#ifndef SHIFT_RESOURCE_RESOURCE_GROUP_H
#define SHIFT_RESOURCE_RESOURCE_GROUP_H

#include <cstdint>
#include <vector>
#include "shift/resource/resource_ptr.h"

namespace shift::resource
{
class image;
class mesh;

///
class resource_group final : public resource_base
{
public:
  static constexpr resource_type static_type = resource_type::resource_group;

  /// Default constructor.
  resource_group() noexcept : resource_base(static_type)
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
