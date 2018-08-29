#ifndef SHIFT_SERIALIZATION2_POINTER_H
#define SHIFT_SERIALIZATION2_POINTER_H

#include <memory>
#include "shift/core/group_ptr.h"
#include "shift/serialization2/types.h"

namespace shift::serialization2
{
/// A plain pointer wrapper which has the same semantics like a simple raw
/// pointer. The type is needed to avoid ambiguous (de-)serialization shift
/// operators.
template <typename T>
class raw_ptr
{
public:
  /// Constructor.
  inline raw_ptr(T* ptr = nullptr) : _ptr(ptr)
  {
  }

  /// Destructor.
  inline ~raw_ptr()
  {
#if defined(_DEBUG)
    _ptr = nullptr;
#endif
  }

  /// Copy assignment operator.
  inline raw_ptr& operator=(const raw_ptr& other)
  {
    _ptr = other.get();
    return *this;
  }

  /// Assignment operator.
  inline raw_ptr& operator=(T* ptr)
  {
    _ptr = ptr;
    return *this;
  }

  ///
  operator bool() const
  {
    return _ptr != nullptr;
  }

  ///
  bool operator!() const
  {
    return _ptr == nullptr;
  }

  ///
  inline T& operator*()
  {
    BOOST_ASSERT(_ptr != nullptr);
    return *_ptr;
  }

  ///
  inline T& operator*() const
  {
    BOOST_ASSERT(_ptr != nullptr);
    return *_ptr;
  }

  ///
  inline T* operator->()
  {
    BOOST_ASSERT(_ptr != nullptr);
    return _ptr;
  }

  ///
  inline T* operator->() const
  {
    BOOST_ASSERT(_ptr != nullptr);
    return _ptr;
  }

  ///
  inline T* get()
  {
    return _ptr;
  }

  ///
  inline T* get() const
  {
    return _ptr;
  }

private:
  T* _ptr = nullptr;
};

///
template <boost::endian::order Order, typename U>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         raw_ptr<U>& pointer)
{
  auto cache = archive.cache();
  BOOST_ASSERT(cache);
  if (!cache)
    return archive;

  begin_shared_pointer begin;
  archive >> begin;
  // If the key is null then the object is not present (null).
  if (!begin.key)
  {
    pointer = nullptr;
    archive >> end_shared_pointer{};
    return archive;
  }

  // Check if the object has already been deserialized.
  auto cached_object = cache->find(begin.key);
  BOOST_ASSERT(cached_object);
  if (cached_object)
  {
    pointer = std::static_pointer_cast<U>(cached_object).get();
    archive >> end_shared_pointer{};
    return archive;
  }

  // It hasn't been deserialized, yet, which is not supported for raw
  // pointers.
  pointer = nullptr;
  archive >> end_shared_pointer{};
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const raw_ptr<U> pointer)
{
  auto cache = archive.cache();
  BOOST_ASSERT(cache);
  if (!pointer)
  {
    // Write a null value key to indicate that this object is not present.
    archive << begin_shared_pointer{0} << end_shared_pointer{};
    return archive;
  }

  // Check if the object has already been written to the stream.
  auto key = cache->find(pointer.get());
  if (key)
  {
    // The object is already written to the stream, so only write its key.
    archive << begin_shared_pointer{key} << end_shared_pointer{};
  }
  else
  {
    // The object is not yet in our object cache, which is not supported for
    // raw pointers.
  }
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::unique_ptr<U>& pointer)
{
  begin_raw_pointer begin;
  archive >> begin;
  if (begin.valid)
  {
    pointer = std::make_unique<U>();
    archive >> *pointer;
  }
  archive >> end_raw_pointer{};
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::unique_ptr<U>& pointer)
{
  if (pointer)
  {
    archive << begin_raw_pointer{true};
    archive << *pointer;
  }
  else
    archive << begin_raw_pointer{false};
  archive << end_raw_pointer{};
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::shared_ptr<U>& pointer)
{
  auto cache = archive.cache();
  BOOST_ASSERT(cache);
  if (!cache)
    return archive;

  begin_shared_pointer begin;
  archive >> begin;
  // If the key is null then the object is not present (null).
  if (!begin.key)
  {
    pointer = nullptr;
    archive >> end_shared_pointer{};
    return archive;
  }

  // Check if the object has already been deserialized.
  auto cached_object = cache->find(begin.key);
  if (cached_object)
  {
    pointer = std::static_pointer_cast<U>(cached_object);
    archive >> end_shared_pointer{};
    return archive;
  }

  // It hasn't been deserialized, yet, so do it now.
  pointer = std::make_shared<U>();
  cache->add(begin.key, pointer);
  archive >> *pointer;
  archive >> end_shared_pointer{};
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::shared_ptr<U>& pointer)
{
  auto cache = archive.cache();
  BOOST_ASSERT(cache);
  if (!pointer)
  {
    // Write a null value key to indicate that this object is not present.
    archive << begin_shared_pointer{0} << end_shared_pointer{};
    return archive;
  }

  // Check if the object has already been written to the stream.
  auto key = cache->find(pointer.get());
  if (!key)
  {
    // The object is not yet in our object cache, so we have to write it.
    archive << begin_shared_pointer{cache->add(pointer)} << *pointer;
  }
  else
  {
    // The object is already written to the stream, so only write its key.
    archive << begin_shared_pointer{key};
  }
  archive << end_shared_pointer{};
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::weak_ptr<U>& pointer)
{
  auto cache = archive.cache();
  BOOST_ASSERT(cache);
  if (!cache)
    return archive;

  begin_shared_pointer begin;
  archive >> begin;
  // If the key is null then the object is not present (null).
  if (!begin.key)
  {
    pointer.reset();
    archive >> end_shared_pointer{};
    return archive;
  }

  // Check if the object has already been deserialized.
  auto cached_object = cache->find(begin.key);
  if (cached_object)
  {
    pointer = std::static_pointer_cast<U>(cached_object);
    archive >> end_shared_pointer{};
    return archive;
  }

  // It hasn't been deserialized, yet, so do it now.
  auto object = std::make_shared<U>();
  cache->add(begin.key, object);
  archive >> *object >> end_shared_pointer{};
  pointer = object;
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::weak_ptr<U>& pointer)
{
  auto cache = archive.cache();
  BOOST_ASSERT(cache);
  auto object = pointer.lock();
  if (!object)
  {
    // Write a null value key to indicate that this object is not present.
    archive << begin_shared_pointer{0} << end_shared_pointer{};
    return archive;
  }

  // Check if the object has already been written to the stream.
  auto key = cache->find(object.get());
  if (!key)
  {
    // The object is not yet in our object cache, so we have to write it.
    archive << begin_shared_pointer{cache->add(object)};
    archive << *object;
  }
  else
  {
    // The object is already written to the stream, so only write its key.
    archive << begin_shared_pointer{key};
  }
  archive << end_shared_pointer{};
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         core::group_ptr<U>& pointer)
{
  auto cache = archive.cache();
  BOOST_ASSERT(cache);
  if (!cache)
    return archive;

  begin_shared_pointer begin;
  archive >> begin;
  // If the key is null then the object is not present (null).
  if (!begin.key)
  {
    pointer = core::group_ptr<U>(cache->shared_from_this());
    archive >> end_shared_pointer{};
    return archive;
  }

  // Check if the object has already been deserialized.
  auto cached_object = cache->find(begin.key);
  if (cached_object)
  {
    pointer = core::group_ptr<U>(std::static_pointer_cast<U>(cached_object),
                                 cache->shared_from_this());
    archive >> end_shared_pointer{};
    return archive;
  }

  // It hasn't been deserialized, yet, so do it now.
  auto new_instance = std::make_shared<U>();
  pointer = cache->add(begin.key, new_instance);
  archive >> *new_instance >> end_shared_pointer{};
  return archive;
}

///
template <boost::endian::order Order, typename U>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const core::group_ptr<U>& pointer)
{
  auto cache = archive.cache();
  BOOST_ASSERT(cache);
  auto shared_pointer = *pointer;
  if (!shared_pointer)
  {
    // Write a null value key to indicate that this object is not present.
    archive << begin_shared_pointer{0};
    return archive;
  }

  // Check if the object has already been written to the stream.
  auto key = cache->find(shared_pointer.get());
  if (!key)
  {
    // The object is not yet in our cache, so we have to write it.
    archive << begin_shared_pointer{cache->add(shared_pointer)}
            << *shared_pointer;
  }
  else
  {
    // The object is already written to the stream, so only write its key.
    archive << begin_shared_pointer{key};
  }
  archive << end_shared_pointer{};
  return archive;
}
}

#endif
