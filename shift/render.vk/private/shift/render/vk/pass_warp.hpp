#ifndef SHIFT_RENDER_VK_PASS_WARP_HPP
#define SHIFT_RENDER_VK_PASS_WARP_HPP

#include <array>
#include "shift/render/vk/smart_ptr.hpp"
#include "shift/render/vk/pass.hpp"
#include "shift/render/vk/layer1/descriptor_set_layout.hpp"
#include "shift/render/vk/layer1/pipeline_layout.hpp"
#include "shift/render/vk/layer1/render_pass.hpp"
#include "shift/render/vk/layer1/pipeline.hpp"
#include "shift/render/vk/layer1/sampler.hpp"

namespace shift::render::vk
{
///
class pass_warp : public pass
{
public:
  static constexpr unsigned int patches = 4;
  static constexpr unsigned int rows = 4;
  static constexpr unsigned int columns = 4;

  ///
  pass_warp(vk::layer1::device& device);

  ///
  void create_resource_descriptions();

  ///
  void destroy_resource_descriptions();

  ///
  void create_pipeline(vk::layer1::pipeline_cache& pipeline_cache);

  ///
  void destroy_pipeline();

  ///
  void create_resources(resource_db::scene& scene);

  ///
  void destroy_resources();

  ///
  void update(std::uint32_t buffer_index);

  ///
  vk::layer1::render_pass& render_pass()
  {
    return *_render_pass;
  }

private:
  core::stack_ptr<vk::layer1::descriptor_set_layout> _descriptor_set_layout;
  core::stack_ptr<vk::layer1::pipeline_layout> _pipeline_layout;
  vk::shared_ptr<vk::layer1::render_pass> _render_pass;
  vk::shared_ptr<vk::layer1::pipeline> _pipeline;
  vk::shared_ptr<vk::layer1::sampler> _image_sampler;
};
}

#endif
//#ifndef SHIFT_RENDER_VK_RENDER_PASSES_WARP_INSTANCE_HPP
//#define SHIFT_RENDER_VK_RENDER_PASSES_WARP_INSTANCE_HPP
//
//#include <array>
//#include "shift/render/vk/render_passes/instance.hpp"
//
// namespace shift::render::vk::render_passes
//{
// class render_pass_warp_class;
//
/////
// class render_pass_warp_instance : public render_pass_instance
//{
// public:
//  static constexpr unsigned int patches = 2;
//  static constexpr unsigned int rows = 4;
//  static constexpr unsigned int columns = 4;
//
//  ///
//  render_pass_warp_instance(cube_application& application, vk::layer1::device&
//  device,
//                            render_pass_warp_class& technique);
//
//  ///
//  virtual void pre_update(std::uint32_t buffer_index,
//                          std::chrono::milliseconds frame_time) override;
//
//  ///
//  void update();
//
//  ///
//  virtual void create_resources() override;
//
//  ///
//  virtual void destroy_resources() override;
//
//  ///
//  void create_framebuffers(
//    const std::vector<vk::shared_ptr<vk::layer1::image_view>>&
//    source_image_views, const
//    std::vector<vk::shared_ptr<vk::layer1::image_view>>&
//      target_image_views);
//
//  ///
//  virtual void destroy_framebuffers() override;
//
//  ///
//  virtual void execute(vk::layer1::command_buffer& primary_command_buffer,
//                       std::uint32_t current_buffer,
//                       const vk::rect_2d& render_area) override;
//
// protected:
//  ///
//  virtual void build_command_buffer(std::uint32_t buffer_index) override;
//
// private:
//  struct warp_vertex
//  {
//    math::vector4<float> position;
//    math::vector4<float> textureCoord;
//  };
//
//  using warp_vertices_t = std::array<warp_vertex, patches * rows * columns>;
//
//  struct warp_uniform_data
//  {
//    warp_vertices_t vertices;
//  };
//
//  render_pass_warp_class& _technique;
//
//  vk::shared_ptr<vk::layer1::descriptor_set> _descriptor_set;
//  vk::shared_ptr<vk::layer1::buffer> _uniform_buffer;
//};
//}
//
//#endif
