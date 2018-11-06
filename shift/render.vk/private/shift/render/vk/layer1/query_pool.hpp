#ifndef SHIFT_RENDER_VK_LAYER1_QUERYPOOL_HPP
#define SHIFT_RENDER_VK_LAYER1_QUERYPOOL_HPP

#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;

///
class query_pool
{
public:
  /// Constructor.
  query_pool(vk::layer1::device& device,
             const query_pool_create_info& create_info);

  query_pool(const query_pool&) = delete;
  query_pool(query_pool&&) = delete;

  /// Destructor.
  ~query_pool();

  query_pool& operator=(const query_pool&) = delete;
  query_pool& operator=(query_pool&&) = delete;

  ///
  VkQueryPool& handle() noexcept
  {
    return _query_pool;
  }

  ///
  const VkQueryPool& handle() const noexcept
  {
    return _query_pool;
  }

  ///
  bool results(std::uint32_t first_query, std::uint32_t query_count,
               std::size_t data_size, void* data, VkDeviceSize stride,
               vk::query_result_flags flags);

  ///
  bool results(std::size_t data_size, std::uint32_t* data,
               vk::query_result_flags flags);

  ///
  std::uint32_t size() const;

  ///
  std::uint32_t used() const;

  ///
  std::uint32_t next_index();

  ///
  void reset();

private:
  device* _device = nullptr;
  VkQueryPool _query_pool = nullptr;
  std::uint32_t _size = 0;
  std::uint32_t _used = 0;
};
}

#endif
