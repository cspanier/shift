#ifndef SHIFT_RESOURCE_DB_MOUNTABLE_HPP
#define SHIFT_RESOURCE_DB_MOUNTABLE_HPP

#include <utility>
#include <filesystem>
#include "shift/resource_db/resource.hpp"

namespace shift::resource_db
{
///
class mountable
{
public:
  ///
  mountable(std::filesystem::path path) : _path(std::move(path))
  {
  }

  ///
  virtual ~mountable();

  ///
  virtual bool open(bool read_only = true) = 0;

  ///
  virtual void close() = 0;

  ///
  virtual resource_id lookup_id(const std::filesystem::path& relative_path) = 0;

  ///
  virtual bool load(resource_id id, resource_base& resource,
                    resource_type type) = 0;

  ///
  virtual bool save(const resource_base& resource, resource_type type,
                    resource_id id,
                    const std::filesystem::path& relative_path) = 0;

  ///
  virtual void erase(resource_id id) = 0;

  ///
  const std::filesystem::path& path()
  {
    return _path;
  }

protected:
  std::filesystem::path _path;
};
}

#endif
