#ifndef SHIFT_RESOURCE_DB_BUFFER_HPP
#define SHIFT_RESOURCE_DB_BUFFER_HPP

#include <vector>
#include "shift/resource_db/types.hpp"
#include "shift/resource_db/resource_ptr.hpp"

namespace shift::resource_db
{
template <>
struct resource_traits<buffer>
{
  static constexpr resource_type type_id = resource_type::buffer;
};

/// A buffer points to binary geometry, animation, or skins.
class buffer final : public resource_base
{
public:
  /// Default constructor.
  buffer() noexcept : resource_base(resource_traits<buffer>::type_id)
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
