#include "shift/render/vk/layer1/descriptor_set.h"
#include "shift/render/vk/layer1/descriptor_set_layout.h"
#include "shift/render/vk/layer1/descriptor_pool.h"
#include "shift/render/vk/layer1/device.h"

namespace shift::render::vk::layer1
{
descriptor_set::descriptor_set(
  vk::layer1::device& device,
  vk::shared_ptr<vk::layer1::descriptor_pool> descriptor_pool,
  vk::layer1::descriptor_set_layout& descriptor_set_layout)
: _device(&device), _descriptor_pool(std::move(descriptor_pool))
{
  vk_check(vk::allocate_descriptor_sets(
    _device->handle(),
    core::rvalue_address(vk::descriptor_set_allocate_info(
      /* next */ nullptr,
      /* descriptor_pool */ _descriptor_pool->handle(),
      /* descriptor_set_count */ 1,
      /* set_layouts */ &descriptor_set_layout.handle())),
    &_descriptor_set));
}

descriptor_set::~descriptor_set()
{
  if (_descriptor_pool->supports_free())
  {
    BOOST_ASSERT(_descriptor_pool->handle());
    vk_check(vk::free_descriptor_sets(
      _device->handle(), _descriptor_pool->handle(), 1, &_descriptor_set));
  }
  else
  {
    // Descriptor sets are deleted all at once through the pool destructor.
  }
  _descriptor_set = nullptr;
}
}
