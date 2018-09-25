#ifndef SHIFT_RENDER_VK_CAMERA_H
#define SHIFT_RENDER_VK_CAMERA_H

#include <shift/scene/camera.h>

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
