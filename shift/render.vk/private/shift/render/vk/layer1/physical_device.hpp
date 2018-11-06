#ifndef SHIFT_RENDER_VK_LAYER1_PHYSICALDEVICE_HPP
#define SHIFT_RENDER_VK_LAYER1_PHYSICALDEVICE_HPP

#include <vector>
#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
///
/// @remarks
///   The type is not moveable because several other types hold a pointer to
///   their associated physical_device instance.
class physical_device
{
public:
  /// Constructor.
  explicit physical_device(VkPhysicalDevice handle);

  physical_device(const physical_device&) = delete;
  physical_device(physical_device&&) = delete;

  /// Destructor.
  ~physical_device();

  physical_device& operator=(const physical_device&) = delete;
  physical_device& operator=(physical_device&&) = delete;

  ///
  VkPhysicalDevice& handle() noexcept
  {
    return _physical_device;
  }

  ///
  const VkPhysicalDevice& handle() const noexcept
  {
    return _physical_device;
  }

  ///
  const vk::physical_device_properties& properties() const
  {
    return _properties.properties();
  }

  ///
  const vk::physical_device_idproperties& id_properties() const
  {
    return _id_properties;
  }

  ///
  const vk::physical_device_multiview_properties& multiview_properties()
  {
    return _multiview_properties;
  }

  ///
  const vk::physical_device_features& features() const
  {
    return _features.features();
  }

  const vk::physical_device_multiview_features& multiview_features() const
  {
    return _multiview_features;
  }

  ///
  void queue_family_properties(
    std::vector<vk::queue_family_properties>& properties) const;

  ///
  void format_properties(vk::format format,
                         vk::format_properties& properties) const;

  ///
  void image_format_properties(vk::format format, vk::image_type type,
                               vk::image_tiling tiling,
                               vk::image_usage_flags usage,
                               vk::image_create_flags flags,
                               vk::image_format_properties& properties) const;

private:
  VkPhysicalDevice _physical_device = nullptr;
  vk::physical_device_properties_2 _properties;
  vk::physical_device_idproperties _id_properties;
  vk::physical_device_multiview_properties _multiview_properties;
  vk::physical_device_features_2 _features;
  vk::physical_device_multiview_features _multiview_features;
};
}

#endif
