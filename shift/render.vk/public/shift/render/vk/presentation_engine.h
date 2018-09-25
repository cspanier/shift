#ifndef SHIFT_RENDER_VK_PRESENTATION_ENGINE_H
#define SHIFT_RENDER_VK_PRESENTATION_ENGINE_H

#include <functional>

namespace shift::render::vk
{
/// Interface for custom presentation engines.
/// @remarks
///   This abstract base class is meant for clients implementing their custom
///   presentation engine. This is useful when displaying images through
///   different APIs like OpenGL, rather than using Vulkan's presentation
///   capabilities.
class presentation_engine
{
public:
  /// Destructor.
  virtual ~presentation_engine() = 0;

  ///
  virtual void release_image(std::uint64_t image,
                             std::uint32_t image_index) = 0;

  ///
  virtual void present(std::uint64_t image, std::uint32_t image_index,
                       std::uint32_t wait_semaphore_count,
                       std::uint64_t* wait_semaphores) = 0;

  ///
  virtual std::uint32_t acquire_next_image(std::uint64_t signal_semaphore) = 0;
};
}

#endif
