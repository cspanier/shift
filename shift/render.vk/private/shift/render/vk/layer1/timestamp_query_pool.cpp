#include "shift/render/vk/layer1/timestamp_query_pool.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/renderer_impl.h"

namespace shift::render::vk::layer1
{
timestamp_query_pool::timestamp_query_pool(
  vk::layer1::device& device, const query_pool_create_info& create_info)
: _device(&device)
{
  vk_check(vk::create_query_pool(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_query_pool));
  _buffer.resize(create_info.query_count() * 2);
}

timestamp_query_pool::~timestamp_query_pool()
{
  if (_query_pool != nullptr)
  {
    vk::destroy_query_pool(
      _device->handle(), _query_pool,
      renderer_impl::singleton_instance().default_allocator());
    _query_pool = nullptr;
  }
}

std::uint32_t timestamp_query_pool::size() const
{
  return static_cast<std::uint32_t>(_buffer.size() / 2);
}

bool timestamp_query_pool::copy_results(std::uint32_t first_query,
                                        std::uint32_t query_count,
                                        vk::query_result_flags flags)
{
  BOOST_ASSERT(!flags.test(vk::query_result_flag::_64_bit));
  vk::result result;
  vk_check(result = vk::get_query_pool_results(
             _device->handle(), _query_pool, first_query * 2, query_count * 2,
             _buffer.size() * sizeof(std::uint32_t), _buffer.data(),
             sizeof(std::uint32_t), flags));
  return result == vk::result::success;
}

std::uint32_t timestamp_query_pool::time(std::uint32_t query)
{
  BOOST_ASSERT(query * 2 < _buffer.size());
  return static_cast<std::uint32_t>(
    (_buffer[query * 2 + 1] - _buffer[query * 2]) /
    _device->physical_device().properties().limits().timestamp_period());
}
}
