#ifndef SHIFT_RENDER_VK_UTILITY_HPP
#define SHIFT_RENDER_VK_UTILITY_HPP

#include <cstdint>
#include <shift/resource_db/image.hpp>
#include <shift/resource_db/mesh.hpp>
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/smart_ptr.hpp"

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
vk::format format_from_resource(resource_db::image_format format);

/// Converts a resource vertex component format to Vulkan.
vk::format format_from_resource(
  resource_db::vertex_attribute_data_type data_type,
  resource_db::vertex_attribute_component_type component_type);

/// Convert certain formats that are known to be not supported by hardware
/// to compatible formats.
vk::format device_image_format(vk::format format);

///
vk::shared_ptr<vk::layer1::buffer> copy_image_to_staging_buffer(
  vk::layer1::device& device, resource_db::image& source,
  vk::format destination_format = vk::format::undefined);

///
void copy_image_to_staging_buffer(resource_db::image& source,
                                  vk::layer1::buffer& staging_buffer,
                                  vk::format destination_format,
                                  std::uint32_t mip_level_begin,
                                  std::uint32_t mip_level_end);
}

#endif
