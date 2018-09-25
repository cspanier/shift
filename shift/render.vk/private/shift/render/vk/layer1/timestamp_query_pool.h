#ifndef SHIFT_RENDER_VK_LAYER1_TIMESTAMPQUERYPOOL_H
#define SHIFT_RENDER_VK_LAYER1_TIMESTAMPQUERYPOOL_H

#include "shift/render/vk/shared.h"

namespace shift::render::vk::layer1
{
class device;

///
class timestamp_query_pool
{
public:
  /// Constructor.
  timestamp_query_pool(vk::layer1::device& device,
                       const query_pool_create_info& create_info);

  timestamp_query_pool(const timestamp_query_pool&) = delete;
  timestamp_query_pool(timestamp_query_pool&&) = delete;

  /// Destructor.
  ~timestamp_query_pool();

  timestamp_query_pool& operator=(const timestamp_query_pool&) = delete;
  timestamp_query_pool& operator=(timestamp_query_pool&&) = delete;

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
  std::uint32_t size() const;

  ///
  bool copy_results(std::uint32_t first_query, std::uint32_t query_count,
                    vk::query_result_flags flags);

  ///
  std::uint32_t time(std::uint32_t query);

private:
  device* _device = nullptr;
  VkQueryPool _query_pool = nullptr;
  std::vector<std::uint32_t> _buffer;
};
}

#endif
