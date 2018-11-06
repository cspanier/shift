#ifndef SHIFT_RENDER_VK_LAYER1_PIPELINELAYOUT_HPP
#define SHIFT_RENDER_VK_LAYER1_PIPELINELAYOUT_HPP

#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;

/// @remarks
///   A pipeline layout object is only used during pipeline creation any may
///   be deleted immediately afterwards.
class pipeline_layout
{
public:
  /// Default constructor.
  pipeline_layout() = default;

  /// Constructor.
  pipeline_layout(vk::layer1::device& device,
                  const pipeline_layout_create_info& create_info);

  pipeline_layout(const pipeline_layout&) = delete;
  pipeline_layout(pipeline_layout&&) = delete;

  /// Destructor.
  ~pipeline_layout();

  pipeline_layout& operator=(const pipeline_layout&) = delete;
  pipeline_layout& operator=(pipeline_layout&&) = delete;

  ///
  VkPipelineLayout& handle() noexcept
  {
    return _pipeline_layout;
  }

  ///
  const VkPipelineLayout& handle() const noexcept
  {
    return _pipeline_layout;
  }

private:
  device* _device = nullptr;
  VkPipelineLayout _pipeline_layout = nullptr;
};
}

#endif
