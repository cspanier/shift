#include "shift/render/vk/layer2/model.hpp"
#include "shift/render/vk/renderer_impl.hpp"
#include <gsl/gsl>

#include <shift/log/log.hpp>

namespace shift::render::vk
{
model::~model() noexcept
{
}
}

namespace shift::render::vk::layer2
{
model::model(vk::layer1::device& device,
             boost::intrusive_ptr<vk::layer2::mesh> mesh,
             boost::intrusive_ptr<vk::layer2::material> material)
: _device(&device), _mesh(std::move(mesh)), _material(std::move(material))
{
  if (_mesh != nullptr)
  {
    if (_mesh->add_parent(*this))
      ++_unloaded_children;
  }
  if (_material != nullptr)
  {
    if (_material->add_parent(*this))
      ++_unloaded_children;
  }

  log::debug() << "Model #" << std::hex << reinterpret_cast<std::size_t>(this)
               << " has " << std::dec << _unloaded_children
               << " unavailable dependencies left.";
}

model::~model() noexcept
{
  if (_material != nullptr)
    _material->remove_parent(*this);
  if (_mesh != nullptr)
    _mesh->remove_parent(*this);
}

void model::signal(availability_state state)
{
  if (state == availability_state::unloaded)
  {
    if (++_unloaded_children == 1)
    {
      availability(availability_state::unloaded);
    }
  }
  else if (state == availability_state::loaded)
  {
    BOOST_ASSERT(_unloaded_children > 0);
    if (--_unloaded_children == 0)
    {
      update_resources();
      availability(availability_state::loaded);
    }
  }
  else if (state == availability_state::updated)
  {
    if (_unloaded_children == 0)
    {
      update_resources();
      availability(availability_state::updated);
    }
  }
  log::debug() << "Model #" << std::hex << reinterpret_cast<std::size_t>(this)
               << " now has " << std::dec << _unloaded_children
               << " unavailable dependencies left.";
}

void model::update_resources()
{
  auto& renderer = renderer_impl::singleton_instance();
  for (std::uint32_t frame_index = 0u; frame_index < config::frame_count;
       ++frame_index)
  {
    auto& descriptor_set = gsl::at(_descriptor_sets, frame_index);

    descriptor_set = vk::make_framed_shared<vk::layer1::descriptor_set>(
      *_device, renderer.descriptor_pool(),
      renderer.scene_descriptor_set_layout());

    //    // Prepare descriptor set.
    //    _device->update_descriptor_set(vk::write_descriptor_set(
    //      /* next */ nullptr,
    //      /* dst_set */ descriptor_set->handle(),
    //      /* dst_binding */ 0,
    //      /* dst_array_element */ 0,
    //      /* descriptor_count */ 1,
    //      /* descriptor_type */ vk::descriptor_type::uniform_buffer,
    //      /* image_info */ nullptr,
    //      /* buffer_info */
    //      core::rvalue_address(vk::descriptor_buffer_info(
    //        /* buffer */ gsl::at(_scene_uniform_buffers,
    //        frame_index)->handle(),
    //        /* offset */ 0,
    //        /* range */ gsl::at(_scene_uniform_buffers,
    //        frame_index)->size())),
    //      /* texel_buffer_view */ nullptr));
    //    _device->update_descriptor_set(vk::write_descriptor_set(
    //      /* next */ nullptr,
    //      /* dst_set */ descriptor_set->handle(),
    //      /* dst_binding */ 1,
    //      /* dst_array_element */ 0,
    //      /* descriptor_count */ 1,
    //      /* descriptor_type */ vk::descriptor_type::sampled_image,
    //      /* image_info */
    //      core::rvalue_address(vk::descriptor_image_info(
    //        /* sampler */ nullptr,
    //        /* image_view */
    //        model->material->albedo.first->image_view->handle(),
    //        /* image_layout */ vk::image_layout::shader_read_only_optimal)),
    //      /* buffer_info */ nullptr,
    //      /* texel_buffer_view */ nullptr));
    //    _device->update_descriptor_set(vk::write_descriptor_set(
    //      /* next */ nullptr,
    //      /* dst_set */ descriptor_set->handle(),
    //      /* dst_binding */ 2,
    //      /* dst_array_element */ 0,
    //      /* descriptor_count */ 1,
    //      /* descriptor_type */ vk::descriptor_type::sampler,
    //      /* image_info */
    //      core::rvalue_address(vk::descriptor_image_info(
    //        /* sampler */ model->material->albedo.second->sampler->handle(),
    //        /* image_view */ nullptr,
    //        /* image_layout */ vk::image_layout::shader_read_only_optimal)),
    //      /* buffer_info */ nullptr,
    //      /* texel_buffer_view */ nullptr));
    //    _device->update_descriptor_set(vk::write_descriptor_set(
    //      /* next */ nullptr,
    //      /* dst_set */ descriptor_set->handle(),
    //      /* dst_binding */ 3,
    //      /* dst_array_element */ 0,
    //      /* descriptor_count */ 1,
    //      /* descriptor_type */ vk::descriptor_type::sampled_image,
    //      /* image_info */
    //      core::rvalue_address(vk::descriptor_image_info(
    //        /* sampler */ nullptr,
    //        /* image_view */
    //        model->material->normal.first->image_view->handle(),
    //        /* image_layout */ vk::image_layout::shader_read_only_optimal)),
    //      /* buffer_info */ nullptr,
    //      /* texel_buffer_view */ nullptr));
    //    _device->update_descriptor_set(vk::write_descriptor_set(
    //      /* next */ nullptr,
    //      /* dst_set */ descriptor_set->handle(),
    //      /* dst_binding */ 4,
    //      /* dst_array_element */ 0,
    //      /* descriptor_count */ 1,
    //      /* descriptor_type */ vk::descriptor_type::sampler,
    //      /* image_info */
    //      core::rvalue_address(vk::descriptor_image_info(
    //        /* sampler */ model->material->normal.second->sampler->handle(),
    //        /* image_view */ nullptr,
    //        /* image_layout */ vk::image_layout::shader_read_only_optimal)),
    //      /* buffer_info */ nullptr,
    //      /* texel_buffer_view */ nullptr));
  }
}
}
