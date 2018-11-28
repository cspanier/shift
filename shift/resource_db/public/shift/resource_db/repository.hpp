#ifndef SHIFT_RESOURCE_DB_REPOSITORY_HPP
#define SHIFT_RESOURCE_DB_REPOSITORY_HPP

#include <cstdint>
#include <memory>
#include <stack>
#include <chrono>
#include <boost/filesystem/path.hpp>
#include <shift/core/singleton.hpp>
#include "shift/resource_db/resource.hpp"
#include "shift/resource_db/resource_ptr.hpp"

namespace shift::resource_db
{
class mountable;

/// A repository is a key-value database
class repository : public core::singleton<repository, core::create::on_stack>
{
public:
  /// Default constructor.
  repository();

  /// Destructor.
  ~repository();

  /// Mount a folder or archive file into the repository.
  mountable* mount(boost::filesystem::path path, bool read_only = true);

  ///
  void unmount(mountable* handle);

  /// Manually add a resource to the cache
  void add(std::shared_ptr<resource_base> resource, resource_id id);

  /// Returns the resource identified by path.
  /// @return
  ///   A new default created object is returned if either the resource
  ///   doesn't exist in the manager's database, or if the resource types
  ///   don't match.
  template <typename Resource>
  resource_ptr<Resource> load(const boost::filesystem::path& absolute_path)
  {
    if (auto [new_resource, id] =
          load(absolute_path, resource_traits<Resource>::type_id);
        new_resource)
    {
      // We do have the strong guarantee, that the object returned by load is
      // infact of type Resource.
      return resource_ptr<Resource>{
        std::static_pointer_cast<Resource>(new_resource), id};
    }
    return {};
  }

  /// Returns the resource identified by id.
  /// @return
  ///   A new default created object is returned if either the resource
  ///   doesn't exist in the manager's database, or if the resource types
  ///   don't match.
  template <typename Resource>
  resource_ptr<Resource> load(resource_id id)
  {
    if (auto resource = load(id, resource_traits<Resource>::type_id).first)
    {
      // We do have the strong guarantee, that the object returned by load is
      // infact of type Resource.
      return {std::static_pointer_cast<Resource>(resource), id};
    }
    return {};
  }

  template <typename Resource>
  bool save(resource_ptr<Resource>& resource,
            const boost::filesystem::path& absolute_path)
  {
    return save(*resource.get(), resource_traits<Resource>::type_id,
                resource.id(), absolute_path);
  }

  /// Implementation for load<Resource>(const boost::filesystem::path&) method.
  std::pair<std::shared_ptr<resource_base>, resource_id> load(
    const boost::filesystem::path& absolute_path, resource_type type);

  /// Implementation for load<Resource>(resource_id) method.
  std::shared_ptr<resource_base> load(resource_id id, resource_type type);

  ///
  bool save(const resource_base& resource, resource_type type, resource_id id,
            const boost::filesystem::path& absolute_path);

private:
  struct impl;
  std::unique_ptr<impl> _impl;
};
}

#endif
