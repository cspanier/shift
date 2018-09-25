#include "shift/render/vk/layer1/pipeline_cache.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/renderer_impl.h"

namespace shift::render::vk::layer1
{
pipeline_cache::pipeline_cache(vk::layer1::device& device,
                               const pipeline_cache_create_info& create_info)
: pipeline_cache()
{
  _device = &device;

  vk_check(vk::create_pipeline_cache(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_pipeline_cache));
}

pipeline_cache::~pipeline_cache()
{
  if (_pipeline_cache != nullptr)
  {
    vk::destroy_pipeline_cache(
      _device->handle(), _pipeline_cache,
      renderer_impl::singleton_instance().default_allocator());
    _pipeline_cache = nullptr;
  }
}
}
