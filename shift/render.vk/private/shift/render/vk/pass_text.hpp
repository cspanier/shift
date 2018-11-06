#ifndef SHIFT_RENDER_VK_PASS_TEXT_HPP
#define SHIFT_RENDER_VK_PASS_TEXT_HPP

#include <shift/math/matrix.hpp>
#include "shift/render/vk/smart_ptr.hpp"
#include "shift/render/vk/pass.hpp"
#include "shift/render/vk/layer1/descriptor_set_layout.hpp"
#include "shift/render/vk/layer1/pipeline_layout.hpp"
#include "shift/render/vk/layer1/render_pass.hpp"
#include "shift/render/vk/layer1/pipeline.hpp"
#include "shift/render/vk/layer1/buffer.hpp"

namespace shift::resource
{
class font;
}

namespace shift::render::vk::layer2
{
class multiview;
}

namespace shift::render::vk
{
///
class pass_text : public pass
{
public:
  struct glyph_vertex
  {
    math::vector2<float> position;
  };

  struct glyph_instance
  {
    math::vector3<float> translation;
    float scale;
    math::vector4<float> color;
  };

  struct glyph_uniform
  {
    math::matrix44<float> mvp;
  };

public:
  static constexpr std::uint32_t max_glyphs = 1024;

  ///
  pass_text(vk::layer1::device& device);

  ///
  void create_resource_descriptions();

  ///
  void destroy_resource_descriptions();

  ///
  void create_pipeline(vk::layer1::pipeline_cache& pipeline_cache);

  ///
  void destroy_pipeline();

  ///
  void create_scene_resources(resource::scene& scene);

  ///
  void destroy_scene_resources();

  ///
  void update(vk::layer2::multiview& multiview, std::uint32_t frame_index);

  ///
  vk::layer1::descriptor_set_layout& descriptor_set_layout()
  {
    return *_descriptor_set_layout;
  }

  ///
  vk::layer1::pipeline_layout& pipeline_layout()
  {
    return *_pipeline_layout;
  }

  ///
  vk::layer1::render_pass& render_pass()
  {
    return *_render_pass;
  }

  ///
  vk::layer1::pipeline& solid_pipeline()
  {
    return *_solid_pipeline;
  }

  ///
  vk::layer1::pipeline& curve_pipeline()
  {
    return *_curve_pipeline;
  }

  resource::font& font_resource()
  {
    return *_font_resource;
  }

  vk::layer1::buffer& vertex_buffer()
  {
    return *_vertex_buffer;
  }

  vk::layer1::buffer& index_buffer()
  {
    return *_index_buffer;
  }

private:
  core::stack_ptr<vk::layer1::descriptor_set_layout> _descriptor_set_layout;
  core::stack_ptr<vk::layer1::pipeline_layout> _pipeline_layout;
  vk::shared_ptr<vk::layer1::render_pass> _render_pass;
  vk::shared_ptr<vk::layer1::pipeline> _solid_pipeline;
  vk::shared_ptr<vk::layer1::pipeline> _curve_pipeline;
  std::shared_ptr<resource::font> _font_resource;
  vk::shared_ptr<vk::layer1::buffer> _vertex_buffer;
  vk::shared_ptr<vk::layer1::buffer> _index_buffer;
};
}

#endif

//#ifndef SHIFT_RENDER_VK_RENDER_PASSES_TEXT_INSTANCE_HPP
//#define SHIFT_RENDER_VK_RENDER_PASSES_TEXT_INSTANCE_HPP
//
//#include "shift/render/vk/render_passes/instance.hpp"
//#include "shift/render/vk/render_passes/text_schematic.hpp"
//
// namespace shift::resource
//{
//  class font;
//}
//
// namespace shift::render::vk::render_passes
//{
// class render_pass_text_class;
//
/////
// class render_pass_text_instance : public render_pass_instance
//{
// public:
//  static constexpr std::uint32_t max_glyphs = 1024;
//
//  ///
//  render_pass_text_instance(cube_application& application, vk::layer1::device&
//  device,
//                            render_pass_text_class& technique);
//
//  ///
//  virtual void pre_update(std::uint32_t buffer_index,
//                          std::chrono::milliseconds frame_time) override;
//
//  ///
//  void update();
//
//  ///
//  void create_buffers();
//
//  ///
//  void destroy_buffers();
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
//  render_pass_text_class& _technique;
//  std::vector<render_pass_text_class::glyph_vertex> vertices;
//  std::vector<render_pass_text_class::glyph_instance> instances;
//
//  vk::shared_ptr<vk::layer1::descriptor_set> _text_descriptor_set;
//  vk::shared_ptr<vk::layer1::buffer> _instance_buffer;
//  vk::shared_ptr<vk::layer1::buffer> _uniform_buffer;
//  vk::shared_ptr<vk::layer1::buffer> _solid_draw_buffer;
//  vk::shared_ptr<vk::layer1::buffer> _curve_draw_buffer;
//
//  std::string _text;
//  std::uint32_t _num_glyphs;
//};
//}
//
//#endif
