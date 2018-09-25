#ifndef SHIFT_RESOURCE_RESOURCE_H
#define SHIFT_RESOURCE_RESOURCE_H

#include <cstdint>
#include <istream>
#include <ostream>
#include <shift/serialization2/types.h>
#include <boost/iostreams/filtering_stream.hpp>

namespace shift::crypto
{
class sha256;
}

namespace shift::resource
{
using resource_id = std::uint64_t;

enum class resource_type : std::uint32_t
{
  undefined,
  resource_group,
  buffer,
  font,
  mesh,
  image,
  shader,
  material_descriptor,
  material,
  scene
};

/// Base class for all resources organized in a repository.
class resource_base
{
public:
  /// Destructor.
  virtual ~resource_base() = 0;

  ///
  resource_type type() const;

  ///
  resource_id id() const;

  ///
  virtual void load(resource_id id,
                    boost::iostreams::filtering_istream& stream) = 0;

  ///
  virtual void save(boost::iostreams::filtering_ostream& stream) const = 0;

  ///
  friend crypto::sha256& operator<<(crypto::sha256& context,
                                    const resource_base& resource);

protected:
  /// Constructor.
  resource_base(resource_type type);

  ///
  virtual void hash(crypto::sha256& context) const = 0;

protected:
  resource_type _type;
  resource_id _id = 0;
};
}

#endif
