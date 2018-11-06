#include "shift/render/vk/smart_ptr.hpp"

namespace shift::render::vk
{
releaseable_wrapper::~releaseable_wrapper() = default;

thread_local std::unique_ptr<shared_object_queue>
  shared_object_queue::_local_queue;

void shared_object_queue::initialize()
{
  _local_queue = std::make_unique<shared_object_queue>();
}

void shared_object_queue::finalize()
{
  _local_queue.reset();
}

shared_object_queue& shared_object_queue::local_queue()
{
  BOOST_ASSERT(_local_queue != nullptr);
  return *_local_queue;
}

void shared_object_queue::queue(releaseable_wrapper& object)
{
  /// ToDo: Benchmark locking.
  std::lock_guard lock(_queue_mutex);
  _queue.push(&object);
}

bool shared_object_queue::collect()
{
  auto* object_queue = _local_queue.get();
  BOOST_ASSERT(object_queue != nullptr);

  /// ToDo: Benchmark locking.
  std::lock_guard lock(object_queue->_queue_mutex);

  bool collected = false;
  while (!object_queue->_queue.empty())
  {
    object_queue->_queue.front()->release();
    object_queue->_queue.pop();
    collected = true;
  }
  return collected;
}

thread_local std::unique_ptr<framed_object_queue>
  framed_object_queue::_local_queue;

void framed_object_queue::initialize()
{
  _local_queue = std::make_unique<framed_object_queue>();
}

void framed_object_queue::finalize()
{
  _local_queue.reset();
}

framed_object_queue& framed_object_queue::local_queue()
{
  BOOST_ASSERT(_local_queue != nullptr);
  return *_local_queue;
}

void framed_object_queue::queue(releaseable_wrapper& object)
{
  /// ToDo: Benchmark locking.
  std::lock_guard lock(_queue_mutex);
  _queues[_swapchain_index].push(&object);
}

bool framed_object_queue::collect(std::uint32_t swapchain_index)
{
  auto* object_queue = _local_queue.get();
  BOOST_ASSERT(object_queue != nullptr);

  /// ToDo: Benchmark locking.
  std::lock_guard lock(object_queue->_queue_mutex);

  object_queue->_swapchain_index = swapchain_index;
  auto& queue = object_queue->_queues[swapchain_index];
  bool collected = false;
  // We must not simply loop until the queue is empty because releasing objects
  // might queue new sub-objects.
  auto elements_to_delete = queue.size();
  while (elements_to_delete > 0)
  {
    queue.front()->release();
    queue.pop();
    collected = true;
    --elements_to_delete;
  }
  return collected;
}

thread_local std::unique_ptr<fenced_object_queue>
  fenced_object_queue::_local_queue;

void fenced_object_queue::initialize()
{
  _local_queue = std::make_unique<fenced_object_queue>();
}

void fenced_object_queue::finalize()
{
  _local_queue.reset();
}

fenced_object_queue& fenced_object_queue::local_queue()
{
  BOOST_ASSERT(_local_queue != nullptr);
  return *_local_queue;
}

void fenced_object_queue::queue(vk::layer1::fence& fence,
                                releaseable_wrapper& object)
{
  /// ToDo: Benchmark locking.
  std::lock_guard lock(_queue_mutex);
  _queue.push(std::make_pair(&fence, &object));
}

bool fenced_object_queue::collect()
{
  auto* object_queue = _local_queue.get();
  BOOST_ASSERT(object_queue != nullptr);

  /// ToDo: Benchmark locking.
  std::lock_guard lock(object_queue->_queue_mutex);
  bool collected = false;
  while (!object_queue->_queue.empty())
  {
    auto [fence, object] = object_queue->_queue.front();
    if (!fence->status())
      break;
    object->release();
    object_queue->_queue.pop();
    collected = true;
  }
  return collected;
}
}
