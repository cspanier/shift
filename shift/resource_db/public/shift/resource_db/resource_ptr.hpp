#ifndef SHIFT_RESOURCE_DB_RESOURCE_PTR_HPP
#define SHIFT_RESOURCE_DB_RESOURCE_PTR_HPP

#include <cstdint>
#include <memory>
#include <boost/assert.hpp>
#include <boost/stacktrace.hpp>
#include <shift/core/exception.hpp>
#include <shift/serialization2/all.hpp>
#include "shift/resource_db/types.hpp"
#include "shift/resource_db/resource.hpp"

namespace shift::resource_db
{
///
class resource_ptr_base
{
protected:
  ///
  static std::shared_ptr<resource_base> load(resource_id id,
                                             resource_type type);
};

///
template <typename T>
class resource_ptr final : public resource_ptr_base
{
public:
  /// Default constructor.
  resource_ptr() = default;

  /// Constructor.
  explicit resource_ptr(std::shared_ptr<T>&& resource, resource_id id)
  : _id(id), _resource(std::move(resource))
  {
  }

  /// Constructor.
  explicit resource_ptr(resource_id id) : _id(id)
  {
  }

  /// Constructor.
  explicit resource_ptr(std::shared_ptr<T>&& resource)
  : _resource(std::move(resource))
  {
    update_id();
  }

  /// Copy constructor.
  resource_ptr(const resource_ptr& other)
  : _id(other._id), _resource(other._resource)
  {
  }

  resource_ptr(resource_ptr&&) noexcept = default;
  ~resource_ptr() noexcept = default;

  /// Copy assignment operator.
  resource_ptr& operator=(const resource_ptr<T>& other)
  {
    _id = other._id;
    _resource = other._resource;
    return *this;
  }

  resource_ptr& operator=(resource_ptr<T>&&) noexcept = default;

  ///
  resource_ptr& operator=(resource_id id)
  {
    if (_id != id)
    {
      _id = id;
      _resource = nullptr;
    }
    return *this;
  }

  ///
  resource_ptr& operator=(std::shared_ptr<T>& resource)
  {
    _resource = resource;
    _id = 0;
    update_id();
    return *this;
  }

  ///
  resource_ptr& operator=(std::shared_ptr<T>&& resource)
  {
    _resource = std::move(resource);
    _id = 0;
    return *this;
  }

  ///
  resource_ptr& operator=(std::nullptr_t)
  {
    _id = 0;
    _resource = nullptr;
    return *this;
  }

  ///
  template <boost::endian::order Order>
  friend serialization2::compact_input_archive<Order>& operator>>(
    serialization2::compact_input_archive<Order>& archive,
    resource_ptr<T>& resource_ptr)
  {
    resource_id id;
    archive >> id;
    resource_ptr.id(id);
    return archive;
  }

  ///
  template <boost::endian::order Order>
  friend serialization2::compact_output_archive<Order>& operator<<(
    serialization2::compact_output_archive<Order>& archive,
    const resource_ptr<T>& resource_ptr)
  {
    archive << resource_ptr.id();
    return archive;
  }

  ///
  T& operator*()
  {
    if (!_resource && _id)
    {
      if (auto resource = load(_id, resource_traits<T>::type_id))
        _resource = std::static_pointer_cast<T>(resource);
    }
    BOOST_ASSERT(_resource);
    if (!_resource)
      SHIFT_THROW_EXCEPTION(core::logic_error());
    return *_resource;
  }

  ///
  T* operator->()
  {
    if (!_resource && _id)
    {
      if (auto resource = load(_id, resource_traits<T>::type_id))
        _resource = std::static_pointer_cast<T>(resource);
    }
    BOOST_ASSERT(_resource);
    if (!_resource)
      SHIFT_THROW_EXCEPTION(core::logic_error());
    return _resource.get();
  }

  ///
  operator bool() const
  {
    return _id || _resource;
  }

  ///
  resource_id id() const
  {
    BOOST_ASSERT(_id != 0 || !_resource);
    if (_resource && _id == 0)
      SHIFT_THROW_EXCEPTION(core::logic_error());
    return _id;
  }

  ///
  void id(resource_id new_id)
  {
    if (new_id != _id)
    {
      _id = new_id;
      _resource.reset();
    }
  }

  ///
  void reset()
  {
    _id = 0;
    _resource.reset();
  }

  ///
  resource_id update_id()
  {
    if (_resource)
      _id = _resource->id();
    return _id;
  }

  ///
  std::shared_ptr<T> get_shared()
  {
    if (!_resource && _id)
    {
      if (auto resource = load(_id, resource_traits<T>::type_id))
        _resource = std::static_pointer_cast<T>(resource);
    }
    BOOST_ASSERT(_resource);
    if (!_resource)
      SHIFT_THROW_EXCEPTION(core::logic_error());
    return _resource;
  }

  ///
  const std::shared_ptr<T>& get_shared() const
  {
    BOOST_ASSERT(_resource);
    if (!_resource)
      SHIFT_THROW_EXCEPTION(core::logic_error());
    return _resource;
  }

private:
  resource_id _id = 0;
  std::shared_ptr<T> _resource;
};
}

#endif
