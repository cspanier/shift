#ifndef SHIFT_RENDER_VK_UTILITY_H
#define SHIFT_RENDER_VK_UTILITY_H

#include <cstdint>
#include <shift/resource/image.h>
#include <shift/resource/mesh.h>
#include "shift/render/vk/shared.h"
#include "shift/render/vk/smart_ptr.h"

namespace shift::render::vk::layer1
{
class buffer;
class device;
}

namespace shift::render::vk
{
/// Returns the size in bytes of a pixel in the given Vulkan format.
std::uint32_t pixel_size(vk::format format);

/// Converts a resource image format to Vulkan.
vk::format format_from_resource(resource::image_format format);

/// Converts a resource vertex component format to Vulkan.
vk::format format_from_resource(
  resource::vertex_attribute_data_type data_type,
  resource::vertex_attribute_component_type component_type);

/// Convert certain formats that are known to be not supported by hardware
/// to compatible formats.
vk::format device_image_format(vk::format format);

///
vk::shared_ptr<vk::layer1::buffer> copy_image_to_staging_buffer(
  vk::layer1::device& device, resource::image& source,
  vk::format destination_format = vk::format::undefined);

///
void copy_image_to_staging_buffer(resource::image& source,
                                  vk::layer1::buffer& staging_buffer,
                                  vk::format destination_format,
                                  std::uint32_t mip_level_begin,
                                  std::uint32_t mip_level_end);
}

#endif
