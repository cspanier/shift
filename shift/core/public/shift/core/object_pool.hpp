#ifndef SHIFT_CORE_OBJECTPOOL_HPP
#define SHIFT_CORE_OBJECTPOOL_HPP

#include <limits>
#include <memory>
#include <bitset>
#include <list>
#include <stack>
#include <shift/platform/assert.hpp>
#include "shift/core/exception.hpp"

namespace shift::core
{
/// The object pool allocates chunks of memory large enough to hold N
/// instances at once, and manages allocation and deallocation.
template <typename T,
          std::size_t N = std::numeric_limits<unsigned long long>::digits>
class object_pool
{
public:
  static_assert(N > 0,
                "object_pool requires at least one object per memory chunk.");

public:
  /// Destructor destroying any object that has not been destroyed, yet.
  ~object_pool();

  /// Create a new object from the pool.
  template <typename... Args>
  T* create(Args&&... args);

  /// Destroy an object previously allocated using create().
  void destroy(T* object) noexcept;

  /// Frees all remaining objects at once.
  void clear();

private:
  struct memory_chunk
  {
    std::aligned_storage_t<sizeof(T), alignof(T)> objects[N];
    std::bitset<N> used;
  };

  /// Allocate memory for a new object of type T.
  T* alloc();

  /// Free memory previously allocated using alloc().
  void free(T* object);

  std::size_t _size = 0;
  std::size_t _capacity = 0;
  std::list<std::unique_ptr<memory_chunk>> _used_chunks;
  std::stack<std::unique_ptr<memory_chunk>> _empty_chunks;
};

template <typename T, std::size_t N>
object_pool<T, N>::~object_pool()
{
  clear();
}

template <typename T, std::size_t N>
template <typename... Args>
T* object_pool<T, N>::create(Args&&... args)
{
  T* object = alloc();
  if (!object)
    BOOST_THROW_EXCEPTION(core::out_of_memory());
  try
  {
    new (object) T(std::forward<Args>(args)...);
  }
  catch (...)
  {
    free(object);
    throw;
  }
  return object;
}

template <typename T, std::size_t N>
void object_pool<T, N>::destroy(T* object) noexcept
{
  if (object)
  {
    object->~T();
    free(object);
  }
}

template <typename T, std::size_t N>
void object_pool<T, N>::clear()
{
  for (auto used_chunk_iter = _used_chunks.begin();
       used_chunk_iter != _used_chunks.end(); ++used_chunk_iter)
  {
    auto& chunk = *used_chunk_iter;
    for (std::size_t index = 0, count = chunk->used.count();
         index < N && count > 0; ++index)
    {
      if (chunk->used.test(index))
      {
        reinterpret_cast<T*>(chunk->objects + index)->~T();
        --count;
      }
    }
    chunk->used.reset();
    _empty_chunks.push(std::move(chunk));
  }
  _used_chunks.clear();
}

template <typename T, std::size_t N>
T* object_pool<T, N>::alloc()
{
  memory_chunk* free_chunk = nullptr;
  if (_size >= _capacity)
  {
    if (_empty_chunks.empty())
    {
      // Create a new chunk.
      auto new_chunk = std::make_unique<memory_chunk>();
      free_chunk = new_chunk.get();
      _used_chunks.emplace_back(std::move(new_chunk));
    }
    else
    {
      // Reuse an existing free chunk.
      auto new_chunk = std::move(_empty_chunks.top());
      _empty_chunks.pop();
      free_chunk = new_chunk.get();
      _used_chunks.emplace_back(std::move(new_chunk));
    }
    _capacity += N;
    BOOST_ASSERT(_size < _capacity);
  }
  std::size_t index = 0;
  if (!free_chunk)
  {
    for (auto& used_chunk : _used_chunks)
    {
      if (!used_chunk->used.all())
      {
        free_chunk = used_chunk.get();
        break;
      }
    }
    BOOST_ASSERT(free_chunk);
    if (!free_chunk)
      return nullptr;
    while (index < N && free_chunk->used.test(index))
      ++index;
    BOOST_ASSERT(index < N);
  }

  free_chunk->used.set(index);
  ++_size;
  return reinterpret_cast<T*>(free_chunk->objects + index);
}

template <typename T, std::size_t N>
void object_pool<T, N>::free(T* object)
{
  for (auto used_chunk_iter = _used_chunks.begin();
       used_chunk_iter != _used_chunks.end(); ++used_chunk_iter)
  {
    auto& used_chunk = *used_chunk_iter;
    if (object >= reinterpret_cast<T*>(used_chunk->objects) &&
        object < reinterpret_cast<T*>(used_chunk->objects + N))
    {
      auto index = (reinterpret_cast<std::size_t>(object) -
                    reinterpret_cast<std::size_t>(used_chunk->objects)) /
                   sizeof(std::aligned_storage_t<sizeof(T), alignof(T)>);
      BOOST_ASSERT(used_chunk->used.test(index));
      used_chunk->used.reset(index);
      if (used_chunk->used.none())
      {
        _empty_chunks.push(std::move(used_chunk));
        _used_chunks.erase(used_chunk_iter);
        _capacity -= N;
      }
      --_size;
      return;
    }
  }

  // The passed object doesn't seem to be allocated from this pool.
  BOOST_ASSERT(false);
}
}

#endif
