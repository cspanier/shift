#ifndef SHIFT_RENDER_VK_LAYER1_PIPELINECACHE_HPP
#define SHIFT_RENDER_VK_LAYER1_PIPELINECACHE_HPP

#include <vector>
#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;

/// @remarks
///   A pipeline cache is only accessed during pipeline creation and may be
///   destroyed immediately afterwards.
class pipeline_cache
{
public:
  /// Default constructor.
  pipeline_cache() = default;

  /// Constructor.
  pipeline_cache(vk::layer1::device& device,
                 const pipeline_cache_create_info& create_info);

  pipeline_cache(const pipeline_cache&) = delete;
  pipeline_cache(pipeline_cache&&) = delete;

  /// Destructor.
  ~pipeline_cache();

  pipeline_cache& operator=(const pipeline_cache&) = delete;
  pipeline_cache& operator=(pipeline_cache&&) = delete;

  ///
  VkPipelineCache& handle() noexcept
  {
    return _pipeline_cache;
  }

  ///
  const VkPipelineCache& handle() const noexcept
  {
    return _pipeline_cache;
  }

private:
  device* _device = nullptr;
  VkPipelineCache _pipeline_cache = nullptr;
};
}

#endif
