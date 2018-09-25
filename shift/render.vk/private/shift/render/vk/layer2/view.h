#ifndef SHIFT_RENDER_VK_LAYER2_VIEW_H
#define SHIFT_RENDER_VK_LAYER2_VIEW_H

#include <cstdint>
#include <shift/core/stack_ptr.h>
#include <shift/math/vector.h>
#include <shift/math/matrix.h>
#include "shift/render/vk/smart_ptr.h"
#include "shift/render/vk/layer1/semaphore.h"
#include "shift/render/vk/layer1/command_buffer.h"
#include "shift/render/vk/layer1/image.h"
#include "shift/render/vk/layer1/image_view.h"
#include "shift/render/vk/layer1/framebuffer.h"

namespace shift::render::vk
{
class context_impl;
}

namespace shift::render::vk::layer2
{
class camera;
class multiview;

/// A class encapsulating everything needed to render a specific camera's view
/// to an image of set dimensions.
/// @remarks
///   A view is usually blitted to a window's surface for presentation.
class view final
{
public:
  vk::layer2::multiview* multiview = nullptr;
  math::vector2<float> relative_position;
  vk::layer2::camera* camera = nullptr;

  struct
  {
    math::matrix44<float> last_inv_view_proj;
  } reproject_depth;
};

///
class multiview
{
public:
  ///
  void create_frame_data(math::vector2<std::uint32_t> absolute_size,
                         vk::context_impl& context);

  ///
  void destroy_frame_data();

  std::vector<std::unique_ptr<view>> views;
  math::vector2<float> relative_size = {0.0f, 0.0f};

  vk::layer1::semaphore prepared;
  multi_buffered<vk::shared_ptr<vk::layer1::command_buffer>> command_buffers;

  vk::shared_ptr<vk::layer1::image> scene_image;
  vk::shared_ptr<vk::layer1::image_view> scene_image_view;

  vk::shared_ptr<vk::layer1::image> depth_image;
  vk::shared_ptr<vk::layer1::image_view> depth_image_view;

  vk::shared_ptr<vk::layer1::framebuffer> scene_framebuffer;
  vk::shared_ptr<vk::layer1::framebuffer> text_framebuffer;
  // vk::shared_ptr<vk::layer1::framebuffer> warp_framebuffer;

  struct
  {
    vk::shared_ptr<vk::layer1::command_buffer> command_buffer;
    vk::shared_ptr<vk::layer1::descriptor_set> descriptor_set;
    vk::shared_ptr<vk::layer1::buffer> uniform_buffer;
    vk::shared_ptr<vk::layer1::image> r32ui_image;
    vk::shared_ptr<vk::layer1::image_view> r32ui_image_view;
    vk::shared_ptr<vk::layer1::image> debug_image;
    // vk::shared_ptr<vk::layer1::image_view> debug_image_view;
    vk::shared_ptr<vk::layer1::image> depth_image;
    // vk::shared_ptr<vk::layer1::image_view> depth_image_view;
    vk::layer1::semaphore finished;
  } reproject_depth;

  struct
  {
    vk::shared_ptr<vk::layer1::command_buffer> command_buffer;
  } primary_occludee;

  struct
  {
  } false_negative;
};
}

#endif
