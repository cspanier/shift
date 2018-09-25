#ifndef SHIFT_RESOURCE_BUFFER_H
#define SHIFT_RESOURCE_BUFFER_H

#include <vector>
#include "shift/resource/resource_ptr.h"

namespace shift::resource
{
/// A buffer points to binary geometry, animation, or skins.
class buffer final : public resource_base
{
public:
  static constexpr resource_type static_type = resource_type::buffer;

  /// Default constructor.
  buffer() noexcept : resource_base(static_type)
  {
  }

  /// @see resource_base::load.
  void load(resource_id id, boost::iostreams::filtering_istream& stream) final;

  /// @see resource_base::save.
  void save(boost::iostreams::filtering_ostream& stream) const final;

public:
  std::vector<std::byte> storage;

protected:
  /// @see resource_base::hash.
  void hash(crypto::sha256& context) const final;
};
}

#endif
