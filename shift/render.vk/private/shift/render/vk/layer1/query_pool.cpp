#include "shift/render/vk/layer1/query_pool.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"

namespace shift::render::vk::layer1
{
query_pool::query_pool(vk::layer1::device& device,
                       const query_pool_create_info& create_info)
: _device(&device)
{
  vk_check(vk::create_query_pool(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_query_pool));
  _size = create_info.query_count();
  _used = 0;
}

query_pool::~query_pool()
{
  if (_query_pool != nullptr)
  {
    vk::destroy_query_pool(
      _device->handle(), _query_pool,
      renderer_impl::singleton_instance().default_allocator());
    _query_pool = nullptr;
  }
}

bool query_pool::results(std::uint32_t first_query, std::uint32_t query_count,
                         std::size_t data_size, void* data, VkDeviceSize stride,
                         vk::query_result_flags flags)
{
  vk::result result;
  vk_check(result = vk::get_query_pool_results(_device->handle(), _query_pool,
                                               first_query, query_count,
                                               data_size, data, stride, flags));
  return result == vk::result::success;
}

bool query_pool::results(std::size_t data_size, std::uint32_t* data,
                         vk::query_result_flags flags)
{
  vk::result result;
  BOOST_ASSERT(data);
  BOOST_ASSERT(!flags.test(vk::query_result_flag::_64_bit));
  vk_check(result = vk::get_query_pool_results(
             _device->handle(), _query_pool, 0, _used,
             data_size * sizeof(std::uint32_t), data, 0, flags));
  return result == vk::result::success;
}

std::uint32_t query_pool::size() const
{
  return _size;
}

std::uint32_t query_pool::used() const
{
  return _used;
}

std::uint32_t query_pool::next_index()
{
  return _used++;
}

void query_pool::reset()
{
  _used = 0;
}
}
