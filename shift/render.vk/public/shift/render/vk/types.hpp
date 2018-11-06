#ifndef SHIFT_RENDER_VK_TYPES_HPP
#define SHIFT_RENDER_VK_TYPES_HPP

namespace shift::render::vk::layer1
{
class physical_device;
class device;
class image;
class sampler;
}

namespace shift::render::vk::layer2
{
class view;
class multiview;
class world;
class camera;
}

namespace shift::render::vk
{
class presentation_engine;
class window;
class context;
class buffer;
class texture;
class material;
class mesh;
class model;

using physical_device = vk::layer1::physical_device;
using device = vk::layer1::device;
using image = vk::layer1::image;
using view = vk::layer2::view;
using multiview = vk::layer2::multiview;
using world = vk::layer2::world;
using camera = vk::layer2::camera;
}

#endif
