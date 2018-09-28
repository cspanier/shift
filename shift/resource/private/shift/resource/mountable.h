#ifndef SHIFT_RESOURCE_MOUNTABLE_H
#define SHIFT_RESOURCE_MOUNTABLE_H

#include <boost/filesystem/path.hpp>
#include <utility>
#include "shift/resource/resource.h"

namespace shift::resource
{
///
class mountable
{
public:
  ///
  mountable(boost::filesystem::path path) : _path(std::move(path))
  {
  }

  ///
  virtual ~mountable();

  ///
  virtual bool open(bool read_only = true) = 0;

  ///
  virtual void close() = 0;

  ///
  virtual resource_id lookup_id(
    const boost::filesystem::path& relative_path) = 0;

  ///
  virtual bool load(resource_id id, resource_base& resource,
                    resource_type type) = 0;

  ///
  virtual bool save(const resource_base& resource, resource_type type,
                    resource_id id,
                    const boost::filesystem::path& relative_path) = 0;

  ///
  virtual void erase(resource_id id) = 0;

  ///
  const boost::filesystem::path& path()
  {
    return _path;
  }

protected:
  boost::filesystem::path _path;
};
}

#endif
