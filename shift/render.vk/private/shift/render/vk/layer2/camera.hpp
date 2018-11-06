#ifndef SHIFT_RENDER_VK_CAMERA_HPP
#define SHIFT_RENDER_VK_CAMERA_HPP

#include <shift/scene/camera.hpp>

namespace shift::render::vk::layer2
{
class camera final : public scene::camera_extension
{
public:
  camera(scene::entity& entity);

private:
};
}

#endif
