#include "shift/render/vk/layer1/physical_device.h"
#include <cstdint>

namespace shift::render::vk::layer1
{
physical_device::physical_device(VkPhysicalDevice handle)
: _physical_device(handle)
{
  _properties.next(&_id_properties);
  _id_properties.next(&_multiview_properties);
  vk::get_physical_device_properties_2(_physical_device, &_properties);

  _features.next(&_multiview_features);
  vk::get_physical_device_features_2(_physical_device, &_features);
}

physical_device::~physical_device()
{
  _physical_device = nullptr;
}

void physical_device::queue_family_properties(
  std::vector<vk::queue_family_properties>& properties) const
{
  std::uint32_t queue_count = 0;
  vk::get_physical_device_queue_family_properties(_physical_device,
                                                  &queue_count, nullptr);
  properties.resize(queue_count);
  get_physical_device_queue_family_properties(_physical_device, &queue_count,
                                              properties.data());
}

void physical_device::format_properties(vk::format format,
                                        vk::format_properties& properties) const
{
  vk::get_physical_device_format_properties(_physical_device, format,
                                            &properties);
}

void physical_device::image_format_properties(
  const vk::format format, vk::image_type type, vk::image_tiling tiling,
  vk::image_usage_flags usage, vk::image_create_flags flags,
  vk::image_format_properties& properties) const
{
  vk::get_physical_device_image_format_properties(
    _physical_device, format, type, tiling, usage, flags, &properties);
}
}
