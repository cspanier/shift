#ifndef SHIFT_RESOURCE_REPOSITORY_IMPL_H
#define SHIFT_RESOURCE_REPOSITORY_IMPL_H

#include <set>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <shared_mutex>
#include <shift/core/abstract_factory.h>
#include "shift/resource/repository.h"
#include "shift/resource/mountable.h"

namespace shift::resource
{
struct repository::impl
{
  using resource_factory_t =
    core::abstract_factory<resource_type, resource_base,
                           core::factory_using_shared_ptr>;

  std::shared_mutex mount_point_mutex;
  std::vector<std::unique_ptr<mountable>> mount_points;

  resource_factory_t resource_factory;

  std::shared_mutex cache_mutex;
  std::unordered_map<resource_id, std::weak_ptr<resource_base>> cache;

  static thread_local std::stack<mountable*> target_archives;
};
}

#endif
