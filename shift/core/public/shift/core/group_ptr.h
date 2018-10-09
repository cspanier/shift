#ifndef SHIFT_CORE_GROUPPTR_H
#define SHIFT_CORE_GROUPPTR_H

#include <cstdint>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <shift/platform/assert.h>
#include "shift/core/types.h"
#include "shift/core/core.h"

namespace shift::core
{
/// The group is a support class used to serialize arbitrary graphs of objects
/// (i.e. graphs with circular dependencies). It provides a unique key for
/// each object in the group, which can be used for (de-)serialization.
class group : public std::enable_shared_from_this<group>
{
public:
  using key_t = std::uint16_t;

public:
  /// Default constructor.
  group() = default;

  /// Destructor.
  virtual ~group() = default;

  /// Checks if the passed object already exists in the group and if so,
  /// returns its key. Otherwise a null value key is returned.
  key_t find(void* object)
  {
    for (const auto& element : _cache)
    {
      if (element.second.get() == object)
        return element.first;
    }
    return 0;
  }

  /// Returns the object associated with the passed key, or nullptr if the key
  /// is invalid.
  std::shared_ptr<void> find(key_t key)
  {
    auto iterator = _cache.find(key);
    if (iterator == _cache.end())
      return nullptr;
    else
      return iterator->second;
  }

  /// Adds an object to the group and returns its key.
  key_t add(const std::shared_ptr<void>& object)
  {
    BOOST_ASSERT(object);
    key_t key = ++_last_key;
    BOOST_ASSERT(key != 0);
    _cache[key] = object;
    return key;
  }

  template <typename T>
  group_ptr<T> add(key_t key, const std::shared_ptr<T>& object)
  {
    BOOST_ASSERT(object);
    BOOST_ASSERT(key != 0);
    BOOST_ASSERT(_cache.find(key) == _cache.end());
    ++_last_key;
    BOOST_ASSERT(key == _last_key);
    _cache[key] = object;
    return group_ptr<T>(object, shared_from_this());
  }

  /// Adds all objects from another group to this group.
  void add(const group& other)
  {
    for (auto [key, object_ptr] : other._cache)
      add(object_ptr);
  }

  /// Returns the number of objects in the group.
  std::size_t size()
  {
    return _cache.size();
  }

private:
  std::atomic<key_t> _last_key = ATOMIC_VAR_INIT(0);
  std::unordered_map<key_t, std::shared_ptr<void>> _cache;
};

/// A group_ptr is a special smart pointer which holds a reference to an
/// object which is part of an arbitrary graph of objects which support cyclic
/// dependencies. Once all group_ptr instances are destroyed, the whole graph
/// will be automatically deleted.
template <typename T>
class group_ptr
{
public:
  using object = T;
  using key_t = typename core::group::key_t;

public:
  /// Default constructor.
  group_ptr() = default;

  group_ptr(std::nullptr_t)
  {
  }

  /// Constructor
  group_ptr(std::shared_ptr<core::group> group) : _group(group)
  {
  }

  /// Constructor
  group_ptr(std::shared_ptr<core::group> group, const std::shared_ptr<T>& node)
  : _node(node), _group(group)
  {
    BOOST_ASSERT(!node || group);
    if (group && node)
      group->add(std::move(node));
  }

  /// Constructor.
  group_ptr(const std::shared_ptr<core::group>& group, key_t key)
  : _node(std::static_pointer_cast<T>(group->find(key))), _group(group)
  {
  }

  /// Constructor.
  group_ptr(std::shared_ptr<T> node, std::shared_ptr<core::group> group)
  : _node(std::move(node)), _group(group)
  {
  }

  /// Copy constructor.
  group_ptr(const group_ptr& other) : _node(other._node), _group(other._group)
  {
  }

  /// Move constructor.
  group_ptr(group_ptr&& other) noexcept
  : _node(std::move(other._node)), _group(std::move(other._group))
  {
  }

  /// Copy assignment operator.
  group_ptr& operator=(const group_ptr& other)
  {
    _node = other._node;
    _group = other._group;
    return *this;
  }

  /// Move assignment operator.
  group_ptr& operator=(group_ptr&& other) noexcept
  {
    _node = std::move(other._node);
    _group = std::move(other._group);
    return *this;
  }

  /// Assignment operator resetting the pointer.
  group_ptr& operator=(std::nullptr_t)
  {
    _node.reset();
    _group.reset();
    return *this;
  }

  /// Returns true if the smart pointer points to an instance of an object.
  operator bool() const
  {
    return !_node.expired();
  }

  std::shared_ptr<T> operator*() const
  {
    return _node.lock();
  }

  ///
  std::shared_ptr<T> operator->() const
  {
    return _node.lock();
  }

  /// Returns the object pointed to.
  std::shared_ptr<T> get() const
  {
    return _node.lock();
  }

  /// Returns the shared object group.
  std::shared_ptr<core::group> group() const
  {
    return _group.lock();
  }

private:
  std::weak_ptr<T> _node;
  std::weak_ptr<core::group> _group;
};

template <typename T, typename... Args>
group_ptr<T> make_group_ptr(std::shared_ptr<core::group> group, Args&&... args)
{
  return group_ptr<T>(std::move(group),
                      std::make_shared<T>(std::forward<Args>(args)...));
}
}

#endif
