#include "shift/resource/resource_ptr.hpp"
#include "shift/resource/repository.hpp"

namespace shift::resource
{
std::shared_ptr<resource_base> resource_ptr_base::load(resource_id id,
                                                       resource_type type)
{
  BOOST_ASSERT(id);
  if (id == 0)
    return nullptr;
  return repository::singleton_instance().load(id, type);
}
}
