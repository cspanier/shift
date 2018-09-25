#include "shift/render/vk/layer2/sampler.h"
#include "shift/render/vk/layer1/sampler.h"
#include "shift/render/vk/layer1/device.h"

namespace shift::render::vk
{
sampler::~sampler() noexcept
{
}
}

namespace shift::render::vk::layer2
{
sampler::sampler(vk::layer1::device& device,
                 sampler_address_mode address_mode_u,
                 sampler_address_mode address_mode_v,
                 sampler_address_mode address_mode_w, float max_anisotropy,
                 float min_lod, float max_lod)
: _address_modes({address_mode_u, address_mode_v, address_mode_w}),
  _max_anisotropy(max_anisotropy),
  _min_lod(min_lod),
  _max_lod(max_lod)
{
  _sampler = vk::make_framed_shared<vk::layer1::sampler>(
    device, vk::sampler_create_info(
              /* next */ nullptr,
              /* flags */ vk::sampler_create_flag::none,
              /* mag_filter */ vk::filter::linear,
              /* min_filter */ vk::filter::linear,
              /* mipmap_mode */ vk::sampler_mipmap_mode::linear,
              /* address_mode_u */
              static_cast<vk::sampler_address_mode>(_address_modes.u),
              /* address_mode_v */
              static_cast<vk::sampler_address_mode>(_address_modes.v),
              /* address_mode_w */
              static_cast<vk::sampler_address_mode>(_address_modes.w),
              /* mip_lod_bias */ 0.0f,
              /* anisotropy_enable */ VK_TRUE,
              /* max_anisotropy */ _max_anisotropy,
              /* compare_enable */ VK_FALSE,
              /* compare_op */ vk::compare_op::never,
              /* min_lod */ _min_lod,
              /* max_lod */ _max_lod,
              /* border_color */ vk::border_color::float_transparent_black,
              /* unnormalized_coordinates */ VK_FALSE));
  availability(availability_state::loaded);
}

sampler::~sampler() noexcept
{
  availability(availability_state::unloaded);
}

sampler_address_modes_uvw sampler::address_modes() const
{
  return _address_modes;
}

float sampler::max_anisotropy()
{
  return _max_anisotropy;
}

float sampler::min_lod()
{
  return _min_lod;
}

float sampler::max_lod()
{
  return _max_lod;
}
}
