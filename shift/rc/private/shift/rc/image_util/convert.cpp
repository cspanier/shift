#include "shift/rc/image_util/convert.hpp"
#include "shift/rc/image_util/linear_image_view.hpp"

namespace shift::rc::image_util
{
// template <typename Source>
// struct template_instantiation_level_2
//{
//  template <typename Destination>
//  void operator()(const Destination*) const
//  {
//    convert<typename linear_image_view_r8g8b8a8_unorm::pixel_t,
//            typename linear_image_view_r8g8b8a8_snorm::pixel_t>{}(nullptr,
//                                                                  nullptr);
//  }
//};

// struct template_instantiation_level_1
//{
//  template <typename Source>
//  void operator()(const Source*) const
//  {
//    core::for_each<image_view_types>(template_instantiation_level_2<Source>{});
//  }
//};

// void template_instantiation()
//{
//  core::for_each<image_view_types>(template_instantiation_level_1{});
//}
}
