#include "shift/resource_db/resource.hpp"
#include "shift/resource_db/repository.hpp"
#include <shift/crypto/sha256.hpp>
#include <boost/assert.hpp>

namespace shift::resource_db
{
resource_base::~resource_base() = default;

resource_type resource_base::type() const
{
  return _type;
}

resource_id resource_base::id() const
{
  if (_id == 0)
  {
    crypto::sha256::digest_t digest{};
    crypto::sha256 context(digest);
    hash(context);
    return crypto::reduce<resource_id>(context.finalize());
  }
  else
    return _id;
}

crypto::sha256& operator<<(crypto::sha256& context,
                           const resource_base& resource)
{
  resource.hash(context);
  return context;
}

resource_base::resource_base(resource_type type) : _type(type)
{
}
}
