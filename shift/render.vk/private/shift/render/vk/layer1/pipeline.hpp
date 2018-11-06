#ifndef SHIFT_RENDER_VK_LAYER1_PIPELINE_HPP
#define SHIFT_RENDER_VK_LAYER1_PIPELINE_HPP

#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;
class pipeline_cache;

///
class pipeline
{
public:
  /// Graphics pipeline constructor.
  pipeline(vk::layer1::device& device,
           vk::layer1::pipeline_cache& pipeline_cache,
           const graphics_pipeline_create_info& create_info);

  /// Compute pipeline constructor.
  pipeline(vk::layer1::device& device,
           vk::layer1::pipeline_cache& pipeline_cache,
           const compute_pipeline_create_info& create_info);

  pipeline(const pipeline&) = delete;
  pipeline(pipeline&&) = delete;

  /// Destructor.
  ~pipeline();

  pipeline& operator=(const pipeline&) = delete;
  pipeline& operator=(pipeline&&) = delete;

  ///
  VkPipeline& handle() noexcept
  {
    return _pipeline;
  }

  ///
  const VkPipeline& handle() const noexcept
  {
    return _pipeline;
  }

  /// Returns a reference to the associated pipeline cache.
  vk::layer1::pipeline_cache& pipeline_cache() const
  {
    return *_pipeline_cache;
  }

private:
  device* _device = nullptr;
  vk::layer1::pipeline_cache* _pipeline_cache = nullptr;
  VkPipeline _pipeline = nullptr;
};
}

#endif
