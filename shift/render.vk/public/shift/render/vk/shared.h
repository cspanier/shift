#ifndef SHIFT_RENDER_VK_VULKANINTERFACE_H
#define SHIFT_RENDER_VK_VULKANINTERFACE_H

#if defined(SHIFT_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#if defined(SHIFT_PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(SHIFT_PLATFORM_LINUX)
#if defined(USE_WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
#else
#define VK_USE_PLATFORM_XCB_KHR
#endif
#endif
#include "shift/render/vk/vulkan.h"

#include <array>
#include <vector>
#include <shift/core/boost_disable_warnings.h>
#include <boost/assert.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/exception.h>
#include <shift/core/stack_ptr.h>
#if defined(BUILD_CONFIG_DEBUG)
#include <shift/platform/environment.h>
#include <shift/log/log.h>
#endif

namespace shift::render::vk
{
///
struct config
{
  static constexpr std::size_t max_worker_thread_count = 8;
  static constexpr std::uint32_t max_swapchain_length = 4;
  static constexpr std::uint32_t frame_count = 2;
  static constexpr bool debug_reproject_depth = true;
  static constexpr vk::sample_count_flag multisample_count =
    vk::sample_count_flag::_4_bit;
  static constexpr vk::format depth_format = vk::format::d32_sfloat;
  static constexpr std::size_t max_active_job_count = 32;
};

///
using vulkan_error_code = boost::error_info<struct vulkan_error_code_tag, int>;

#if defined(BUILD_CONFIG_DEBUG)
///
inline void debug_check(vk::result code, const char* command)
{
  BOOST_ASSERT(static_cast<int>(code) >= 0);
  if (static_cast<int>(code) < 0)
  {
    shift::log::warning() << "The command \"" << command
                          << "\" returned an unexpected error code ("
                          << static_cast<int>(code) << ").";
    shift::platform::environment::debug_break();
  }
}

#define vk_check(expression) \
  ::shift::render::vk::debug_check((expression), #expression)
#else
#define vk_check(expression) (void)(expression)
#endif

///
template <typename T>
using swap_data = std::array<T, config::max_swapchain_length>;

///
template <typename T>
using multi_buffered = std::array<T, config::frame_count>;
}

#endif
