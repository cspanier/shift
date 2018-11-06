#include "shift/render/vk/layer1/pipeline.hpp"
#include "shift/render/vk/layer1/pipeline_cache.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"

namespace shift::render::vk::layer1
{
pipeline::pipeline(vk::layer1::device& device,
                   vk::layer1::pipeline_cache& pipeline_cache,
                   const graphics_pipeline_create_info& create_info)
: _device(&device), _pipeline_cache(&pipeline_cache)
{
  vk_check(vk::create_graphics_pipelines(
    _device->handle(), _pipeline_cache->handle(), 1, &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_pipeline));
}

pipeline::pipeline(vk::layer1::device& device,
                   vk::layer1::pipeline_cache& pipeline_cache,
                   const compute_pipeline_create_info& create_info)
: _device(&device), _pipeline_cache(&pipeline_cache)
{
  vk_check(vk::create_compute_pipelines(
    _device->handle(), _pipeline_cache->handle(), 1, &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_pipeline));
}

pipeline::~pipeline()
{
  if (_pipeline != nullptr)
  {
    vk::destroy_pipeline(
      _device->handle(), _pipeline,
      renderer_impl::singleton_instance().default_allocator());
    _pipeline = nullptr;
  }
}
}
