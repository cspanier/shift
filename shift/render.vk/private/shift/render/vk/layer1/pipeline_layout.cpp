#include "shift/render/vk/layer1/pipeline_layout.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"

namespace shift::render::vk::layer1
{
pipeline_layout::pipeline_layout(vk::layer1::device& device,
                                 const pipeline_layout_create_info& create_info)
: pipeline_layout()
{
  _device = &device;

  vk_check(vk::create_pipeline_layout(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(),
    &_pipeline_layout));
}

pipeline_layout::~pipeline_layout()
{
  if (_pipeline_layout != nullptr)
  {
    vk::destroy_pipeline_layout(
      _device->handle(), _pipeline_layout,
      renderer_impl::singleton_instance().default_allocator());
    _pipeline_layout = nullptr;
  }
}
}
