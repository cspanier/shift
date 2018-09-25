#ifndef SHIFT_RENDER_VK_LAYER1_SEMAPHORE_H
#define SHIFT_RENDER_VK_LAYER1_SEMAPHORE_H

#include "shift/render/vk/shared.h"

namespace shift::render::vk::layer1
{
class device;

///
struct semaphore
{
public:
  /// Default constructor creating an uninitialized semaphore.
  semaphore() = default;

  /// Constructor creating an initialized semaphore.
  semaphore(vk::layer1::device& device);

  semaphore(const semaphore&) = delete;

  /// Move constructor.
  semaphore(semaphore&& other) noexcept;

  /// Destructor.
  ~semaphore();

  semaphore& operator=(const semaphore&) = delete;

  /// Move assignment operator.
  semaphore& operator=(semaphore&& other) noexcept;

  /// Checks whether the semaphore is initialized.
  operator bool() const noexcept
  {
    return _semaphore != nullptr;
  }

  ///
  VkSemaphore& handle() noexcept
  {
    return _semaphore;
  }

  ///
  const VkSemaphore& handle() const noexcept
  {
    return _semaphore;
  }

  /// Deletes the semaphore.
  /// @post
  ///   The semaphore is not initialized.
  void reset();

private:
  device* _device = nullptr;
  VkSemaphore _semaphore = nullptr;
};
}

#endif
