#include "shift/resource/repository.h"
#include "shift/resource/repository_impl.h"
#include "shift/resource/filesystem.h"
#include "shift/resource/archive.h"
#include "shift/resource/resource_group.h"
#include "shift/resource/scene.h"
#include "shift/resource/font.h"
#include "shift/resource/shader.h"
#include "shift/resource/image.h"
#include "shift/resource/material.h"
#include <shift/serialization2/all.h>
#include <shift/crypto/sha256.h>
#include <shift/core/substream_device.h>
#include <shift/core/bit_field.h>
#include <shift/core/exception.h>
#include <shift/core/string_util.h>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <map>
#include <array>

namespace shift::resource
{
namespace fs = boost::filesystem;

thread_local std::stack<mountable*> repository::impl::target_archives;

repository::repository() : _impl(std::make_unique<impl>())
{
  _impl->resource_factory.template register_type<resource_group>(
    resource_type::resource_group);
  _impl->resource_factory.template register_type<buffer>(resource_type::buffer);
  _impl->resource_factory.template register_type<font>(resource_type::font);
  _impl->resource_factory.template register_type<mesh>(resource_type::mesh);
  _impl->resource_factory.template register_type<image>(resource_type::image);
  _impl->resource_factory.template register_type<shader>(resource_type::shader);
  _impl->resource_factory.template register_type<scene>(resource_type::scene);
  _impl->resource_factory.template register_type<material>(
    resource_type::material);
}

repository::~repository() = default;

mountable* repository::mount(boost::filesystem::path path, bool read_only)
{
  fs::path mount_path;
  try
  {
    mount_path =
      fs::relative(fs::canonical(fs::absolute(path)), fs::current_path());
  }
  catch (...)
  {
    return nullptr;
  }

  {
    // Check if the location is already mounted.
    std::shared_lock read_lock(_impl->mount_point_mutex);
    for (const auto& mount_point : _impl->mount_points)
    {
      if (mount_point->path() == mount_path)
        return nullptr;
    }
  }

  std::unique_ptr<mountable> mount_point;
  if (fs::is_directory(mount_path))
    mount_point = std::make_unique<filesystem>(mount_path);
  else
    mount_point = std::make_unique<archive>(mount_path);
  if (!mount_point->open(read_only))
    return nullptr;

  auto* mount_point_ptr = mount_point.get();
  std::unique_lock write_lock(_impl->mount_point_mutex);
  _impl->mount_points.push_back(std::move(mount_point));
  return mount_point_ptr;
}

void repository::unmount(mountable* handle)
{
  BOOST_ASSERT(handle);
  if (!handle)
    return;

  std::unique_lock write_lock(_impl->mount_point_mutex);
  for (auto mount_point_iter = begin(_impl->mount_points);
       mount_point_iter != end(_impl->mount_points); ++mount_point_iter)
  {
    if (mount_point_iter->get() == handle)
    {
      _impl->mount_points.erase(mount_point_iter);
      return;
    }
  }
  /// ToDo: Throw exception.
  BOOST_ASSERT(false);
}

void repository::add(std::shared_ptr<resource_base> resource, resource_id id)
{
  std::unique_lock write_lock(_impl->cache_mutex);
  _impl->cache[id] = resource;
}

// resource_id repository::query(const boost::filesystem::path& name)
//{
//  // std::shared_lockread_lock(_impl->mount_point_mutex);
//  // auto id = name_to_id(name);
//  // auto name_iter = _impl->name_mappings.find(id);
//  // if (name_iter != _impl->name_mappings.end())
//  //  return name_iter->second;
//  // else
//  return 0;
//}

// resource_id repository::name_to_id(const boost::filesystem::path& name)
//{
//  crypto::sha256::digest digest;
//  crypto::sha256 context(digest);
//  context << core::to_lower(name.generic_string());
//  return crypto::reduce<resource_id>(context.finalize());
//}

std::pair<std::shared_ptr<resource_base>, resource_id> repository::load(
  const boost::filesystem::path& absolute_path, resource_type type)
{
  auto resource = _impl->resource_factory.create_instance(type);
  std::shared_lock read_lock(_impl->mount_point_mutex);
  for (auto& mount_point : _impl->mount_points)
  {
    fs::path relative_path = fs::relative(absolute_path, mount_point->path());
    if (relative_path.empty() || *relative_path.begin() == "..")
      continue;
    if (auto id = mount_point->lookup_id(relative_path))
    {
      {
        // Check if we already loaded the resource.
        std::shared_lock read_lock(_impl->cache_mutex);
        auto cache_iter = _impl->cache.find(id);
        if (cache_iter != _impl->cache.end())
        {
          if (auto cached_resource = cache_iter->second.lock())
            return {cached_resource, id};
        }
      }

      if (mount_point->load(id, *resource, type))
      {
        BOOST_ASSERT(resource->id() == id);

        std::unique_lock write_lock(_impl->cache_mutex);
        _impl->cache[id] = resource;

        return {resource, id};
      }
    }
  }
  return {nullptr, 0};
}

std::shared_ptr<resource_base> repository::load(resource_id id,
                                                resource_type type)
{
  {
    // Check if we already loaded the resource.
    std::shared_lock read_lock(_impl->cache_mutex);
    auto cache_iter = _impl->cache.find(id);
    if (cache_iter != _impl->cache.end())
    {
      if (auto cached_resource = cache_iter->second.lock())
        return cached_resource;
    }
  }

  // Load the resource from repository.
  auto resource = _impl->resource_factory.create_instance(type);
  std::shared_lock read_lock(_impl->mount_point_mutex);
  for (auto& mount_point : _impl->mount_points)
  {
    if (mount_point->load(id, *resource, type))
    {
      BOOST_ASSERT(resource->id() == id);
      if (resource->id() != id)
      {
        SHIFT_THROW_EXCEPTION(
          core::runtime_error() << core::context_info(
            std::to_string(resource->id()) + " != " + std::to_string(id)));
      }

      std::unique_lock write_lock(_impl->cache_mutex);
      _impl->cache[id] = resource;

      return resource;
    }
  }
  SHIFT_THROW_EXCEPTION(core::runtime_error()
                        << core::context_info(std::to_string(id)));
  return nullptr;
}

bool repository::save(const resource_base& resource, resource_type type,
                      resource_id id,
                      const boost::filesystem::path& absolute_path)
{
  namespace fs = boost::filesystem;

  mountable* target_mount_point = nullptr;
  {
    std::shared_lock read_lock(_impl->mount_point_mutex);
    for (const auto& mount_point : _impl->mount_points)
    {
      const auto& mount_path = mount_point->path();
      auto mount_path_iter = mount_path.begin();
      auto file_path_iter = absolute_path.begin();
      while (mount_path_iter != mount_path.end() &&
             file_path_iter != absolute_path.end())
      {
        if (*mount_path_iter != *file_path_iter)
          break;
        ++mount_path_iter;
        ++file_path_iter;
      }
      if (mount_path_iter == mount_path.end() &&
          file_path_iter != absolute_path.end())
      {
        target_mount_point = mount_point.get();
        break;
      }
    }
    if (!target_mount_point)
      return false;
  }

  boost::system::error_code error_code;
  auto relative_path =
    fs::relative(absolute_path, target_mount_point->path(), error_code);
  if (error_code)
    return false;

  target_mount_point->save(resource, type, id, relative_path);

  return true;
}
}
