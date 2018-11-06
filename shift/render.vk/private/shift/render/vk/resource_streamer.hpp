#ifndef SHIFT_RENDER_VK_RESOURCE_STREAMER_HPP
#define SHIFT_RENDER_VK_RESOURCE_STREAMER_HPP

#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/intrusive_ptr.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/smart_ptr.hpp"
#include "shift/render/vk/layer1/buffer.hpp"
#include "shift/render/vk/layer1/image.hpp"

namespace shift::resource
{
class image;
}

namespace shift::render::vk::layer1
{
class device;
class buffer;
}

namespace shift::render::vk::layer2
{
class texture;
}

namespace shift::render::vk
{
/// This class manages asynchronous upload of buffer and image data using a
/// separate device queue.
/// @remarks
///   We use a single large staging buffer of staging_buffer_size bytes.
///   This buffer uses non-coherent shared memory that is permanently mapped
///   into host address space.
class resource_streamer
{
public:
  static constexpr std::size_t staging_buffer_size = 64 * 1024 * 1024;

public:
  /// Constructor.
  resource_streamer(vk::layer1::device& device);

  /// Destructor.
  ~resource_streamer();

  /// Creates device dependent resources.
  void create_resources();

  /// Destroys all device dependent resources.
  void destroy_resources();

private:
  vk::layer1::device* _device = nullptr;
  vk::shared_ptr<vk::layer1::buffer> _staging_buffer;
  void* _staging_memory = nullptr;
};
}

#endif
