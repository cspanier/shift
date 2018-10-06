#ifndef SHIFT_RENDER_VK_SMART_PTR_H
#define SHIFT_RENDER_VK_SMART_PTR_H

#include <utility>
#include <memory>
#include <queue>
#include <atomic>
#include <mutex>
#include <shift/core/object_pool.h>
#include "shift/render/vk/shared.h"
#include "shift/render/vk/smart_ptr.h"
#include "shift/render/vk/layer1/fence.h"

namespace shift::render::vk
{
// create:
// using new T()
// using make_shared<T>
// using make_unique<T>
// using global thread-safe object_pool<T>
// using thread_local object_pool<T>

// destroy:
// wait for embedded fence object
//  - assign thread_local allocator object to allow deletion from different
//    thread
//  - requires manager that takes ownership and regularly polls fence
//  - on delete put embedded fence and object into queue
//  - each thread loop check queue<pair<fence, T*>>
// wait for external fence object
//  - assign thread_local allocator object to allow deletion from different
//    thread
//  - on delete if fence != 0 put assigned fence and object into queue
//  - on delete if fence == 0 delete object immediately
//  - each thread loop check queue<pair<fence, T*>>
// wait for frame count to increase
//  - only valid on main render thread or threads synced to main render thread
//  - requires manager that takes ownership and regularly increases frame
//    counter
//  - on delete put object in queue[current_swapchain_index]
//  - on new frame AFTER waiting for swapchain image to become available delete
//    all objects in queue[current_swapchain_index]

// shared_ptr vs. intrusive_ptr
// - objects with embedded counter can be turned into intrusive_ptrs back again
// - objects created with make_shared_ptr can only be turned into shared_ptrs
//   when deriving from shared_from_this (which embeds the ref counter into the
//   object)
// - shared_ptr is problematic in library interfaces

// -----------------------------------------------------------------------------

// thread_local object_pool<fenced_shared_wrapper<T>>
// - local allocate
// - local deallocate
// - local object count to prevent thread end before all objects collected
// - no need to access from different thread

// thread_local fenced_object_queue
// - queue<pair<VkFence, shared_wrapper_base*>>
// - off-thread object enqueue
// - requires mutex
// - local fence check and call shared_wrapper_base::release if fence is
// signaled

// shared_wrapper_base
// - ref_count
// - virtual add_ref
// - virtual release

// shared_wrapper<T> : public shared_wrapper_base
// - virtual release override
//  - if ref_count-- == 1: object_pool<fenced_shared_wrapper<T>>::deallocate
// - T object

// fenced_shared_wrapper<T> : public shared_wrapper<T>
// - ctor increments ref_count by one
// - virtual release override
//  - if ref_count-- == 2: put fence and shared_wrapper_base* onto queue
//  - if ref_count-- == 1: object_pool<fenced_shared_wrapper<T>>::deallocate
// - own or external fence
// - pointer to the queue belonging to the thread the object was created from

// shared_ptr<T>
// - add_ref
// - release
// - shared_wrapper<T>*

///
template <typename T>
class object_pool
{
public:
  ///
  static void initialize_pool()
  {
    _pool = std::make_unique<core::object_pool<T>>();
  }

  ///
  static void finalize_pool()
  {
    _pool.reset();
  }

  /// Creates a new object from the current thread's local memory pool.
  template <typename... Ts>
  static T* create(Ts&&... args)
  {
    BOOST_ASSERT(_pool != nullptr);
    auto& pool = *_pool;
    auto* object = pool.create(std::forward<Ts>(args)...);
    BOOST_ASSERT(object != nullptr);
    // ++_instance_count;
    return object;
  }

  ///
  static void destroy(T* object)
  {
    if (!object)
      return;

    BOOST_ASSERT(_pool != nullptr);
    auto& pool = *_pool;
    pool.destroy(object);
  }

private:
  static thread_local std::unique_ptr<core::object_pool<T>> _pool;
};

template <typename T>
thread_local std::unique_ptr<core::object_pool<T>> object_pool<T>::_pool;

///
class releaseable_wrapper
{
public:
  ///
  virtual ~releaseable_wrapper() = 0;

  /// Releases ownership and eventually deletes the object.
  virtual void release() = 0;
};

///
class shared_object_queue
{
public:
  ///
  static void initialize();

  ///
  static void finalize();

  ///
  static shared_object_queue& local_queue();

  ///
  void queue(releaseable_wrapper& object);

  ///
  static bool collect();

private:
  std::recursive_mutex _queue_mutex;
  std::queue<releaseable_wrapper*> _queue;

  static thread_local std::unique_ptr<shared_object_queue> _local_queue;
};

///
class framed_object_queue
{
public:
  ///
  static void initialize();

  ///
  static void finalize();

  ///
  static framed_object_queue& local_queue();

  ///
  void queue(releaseable_wrapper& object);

  ///
  static bool collect(std::uint32_t swapchain_index);

private:
  std::recursive_mutex _queue_mutex;
  std::array<std::queue<releaseable_wrapper*>, config::max_swapchain_length>
    _queues;
  std::uint32_t _swapchain_index = 0;

  static thread_local std::unique_ptr<framed_object_queue> _local_queue;
};

///
class fenced_object_queue
{
public:
  ///
  static void initialize();

  ///
  static void finalize();

  ///
  static fenced_object_queue& local_queue();

  ///
  void queue(vk::layer1::fence& fence, releaseable_wrapper& object);

  ///
  static bool collect();

private:
  std::recursive_mutex _queue_mutex;
  std::queue<std::pair<vk::layer1::fence*, releaseable_wrapper*>> _queue;

  static thread_local std::unique_ptr<fenced_object_queue> _local_queue;
};

///
template <typename T>
class shared_wrapper_base : public releaseable_wrapper
{
public:
  ///
  template <typename... Args>
  shared_wrapper_base(std::uint32_t initial_reference_count, Args&&... args)
  : _reference_counter ATOMIC_VAR_INIT(initial_reference_count),
    _object(std::forward<Args>(args)...)
  {
  }

  ///
  ~shared_wrapper_base() override = default;

  /// Increments the object's reference counter.
  /// @remarks
  ///   This does not have to be virtual for now.
  void add_reference()
  {
    _reference_counter.fetch_add(1, std::memory_order_relaxed);
  }

  ///
  T& get()
  {
    return _object;
  }

  ///
  const T& get() const
  {
    return _object;
  }

protected:
  std::atomic<std::uint32_t> _reference_counter;
  T _object;
};

///
template <typename T>
class shared_wrapper : public shared_wrapper_base<T>
{
public:
  ///
  template <typename... Args>
  shared_wrapper(Args&&... args)
  : shared_wrapper_base<T>(2u, std::forward<Args>(args)...),
    _queue(shared_object_queue::local_queue())
  {
  }

  ///
  void release() override
  {
    switch (shared_wrapper_base<T>::_reference_counter.fetch_sub(
      1, std::memory_order_release))
    {
    case 2:
      // Because we incremented the counter at contruction nobody owns the
      // object at this point any more. Transfer ownership to the
      // shared_object_queue which handles cleaning up objects from previous
      // frames and does the final call to release().
      std::atomic_thread_fence(std::memory_order_acquire);
      _queue.queue(*this);
      break;

    case 1:
      std::atomic_thread_fence(std::memory_order_acquire);
      vk::object_pool<shared_wrapper<T>>::destroy(this);
      break;

    default:
      // NOP.
      break;
    }
  }

protected:
  shared_object_queue& _queue;
};

///
template <typename T>
class framed_shared_wrapper : public shared_wrapper_base<T>
{
public:
  ///
  template <typename... Args>
  framed_shared_wrapper(Args&&... args)
  : shared_wrapper_base<T>(2u, std::forward<Args>(args)...),
    _queue(framed_object_queue::local_queue())
  {
  }

  ///
  void release() override
  {
    switch (shared_wrapper_base<T>::_reference_counter.fetch_sub(
      1, std::memory_order_release))
    {
    case 2:
      // Only the current frame owns the object. Transfer ownership to the
      // framed_object_queue which handles cleaning up objects from previous
      // frames and does the final call to release().
      std::atomic_thread_fence(std::memory_order_acquire);
      _queue.queue(*this);
      break;

    case 1:
      std::atomic_thread_fence(std::memory_order_acquire);
      vk::object_pool<framed_shared_wrapper<T>>::destroy(this);
      break;

    default:
      // NOP.
      break;
    }
  }

protected:
  framed_object_queue& _queue;
};

///
template <typename T>
class fenced_shared_wrapper : public shared_wrapper_base<T>
{
public:
  ///
  template <typename... Args>
  fenced_shared_wrapper(vk::layer1::fence fence, Args&&... args)
  : shared_wrapper_base<T>(2u, std::forward<Args>(args)...),
    _fence(std::move(fence)),
    _queue(fenced_object_queue::local_queue())
  {
  }

  ///
  void release() override
  {
    switch (shared_wrapper_base<T>::_reference_counter.fetch_sub(
      1, std::memory_order_release))
    {
    case 2:
      // Only the fence owns the object. Transfer ownership to the
      // fenced_object_queue which handles fence polling and do the final call
      // to release().
      std::atomic_thread_fence(std::memory_order_acquire);
      _queue.queue(_fence, *this);
      break;

    case 1:
      std::atomic_thread_fence(std::memory_order_acquire);
      vk::object_pool<fenced_shared_wrapper<T>>::destroy(this);
      break;

    default:
      // NOP.
      break;
    }
  }

protected:
  vk::layer1::fence _fence;
  fenced_object_queue& _queue;
};

///
template <typename T>
class shared_ptr
{
public:
  shared_ptr() = default;

  /// Constructor from nullptr.
  explicit shared_ptr(std::nullptr_t) : shared_ptr()
  {
  }

  /// Constructor from shared_wrapper_base<T> used by make*shared.
  shared_ptr(shared_wrapper_base<T>* object) : _wrapper(object)
  {
  }

  /// Copy constructor.
  shared_ptr(const shared_ptr& other) : _wrapper(other._wrapper)
  {
    if (_wrapper)
      _wrapper->add_reference();
  }

  shared_ptr(shared_ptr&& other) : _wrapper(other._wrapper)
  {
    other._wrapper = nullptr;
  }

  /// Destructor.
  ~shared_ptr()
  {
    reset();
  }

  /// Copy assignment operator.
  shared_ptr& operator=(const shared_ptr& other)
  {
    if (_wrapper)
      _wrapper->release();
    _wrapper = other._wrapper;
    if (_wrapper)
      _wrapper->add_reference();
  }

  shared_ptr& operator=(shared_ptr&& other)
  {
    _wrapper = other._wrapper;
    other._wrapper = nullptr;
    return *this;
  }

  ///
  T& operator*()
  {
    return _wrapper->get();
  }

  ///
  T& operator*() const
  {
    return _wrapper->get();
  }

  ///
  T* operator->()
  {
    return &_wrapper->get();
  }

  ///
  T* operator->() const
  {
    return &_wrapper->get();
  }

  ///
  operator bool() const
  {
    return _wrapper != nullptr;
  }

  ///
  T* get()
  {
    return &_wrapper->get();
  }

  ///
  T* get() const
  {
    return &_wrapper->get();
  }

  ///
  void reset()
  {
    if (_wrapper)
    {
      _wrapper->release();
      _wrapper = nullptr;
    }
  }

private:
  shared_wrapper_base<T>* _wrapper = nullptr;
};

template <typename T, typename... Args>
inline shared_ptr<T> make_shared(Args&&... args)
{
  return shared_ptr<T>(
    vk::object_pool<shared_wrapper<T>>::create(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
inline shared_ptr<T> make_framed_shared(Args&&... args)
{
  return shared_ptr<T>(vk::object_pool<framed_shared_wrapper<T>>::create(
    std::forward<Args>(args)...));
}

template <typename T, typename... Args>
inline shared_ptr<T> make_fenced_shared(vk::layer1::fence fence, Args&&... args)
{
  return shared_ptr<T>(vk::object_pool<fenced_shared_wrapper<T>>::create(
    std::move(fence), std::forward<Args>(args)...));
}
}

#endif
