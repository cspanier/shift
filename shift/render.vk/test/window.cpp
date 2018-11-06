#include <chrono>
#include <shift/log/log_server.hpp>
#include <shift/math/vector.hpp>
#include <shift/core/exception.hpp>
#include <shift/core/algorithm.hpp>
#include <shift/core/at_exit_scope.hpp>
#include <shift/platform/environment.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <array>

// struct init_current_working_path
//{
//  init_current_working_path() noexcept
//  {
//    using namespace boost::filesystem;
//    try
//    {
//      current_path(path(shift::platform::environment::executable_path())
//                     .parent_path()
//                     .parent_path());
//    }
//    catch (...)
//    {
//      std::terminate();
//    }
//  }
//};
//
// static init_current_working_path init;
//
// namespace shift
//{
// namespace render
//{
//  namespace vk
//  {
//    struct swapchain_element
//    {
//      swapchain_element() = default;
//
//      swapchain_element(swapchain_element&&) = default;
//
//      swapchain_element& operator=(swapchain_element&&) = default;
//
//      boost::intrusive_ptr<vk::image> image;
//      boost::intrusive_ptr<vk::image_view> image_view;
//      boost::intrusive_ptr<vk::framebuffer> framebuffer;
//    };
//
//    class test_application
//    {
//    public:
//      void run();
//
//    protected:
//      ///
//      virtual const physical_device& select_physical_device(
//        const std::vector<physical_device>& devices);
//
//      ///
//      void create_resources();
//
//      ///
//      void destroy_resources();
//
//      ///
//      void resize_window();
//
//      ///
//      void create_init_command_buffer();
//
//      ///
//      void destroy_init_command_buffer();
//
//      ///
//      void flush_init_command_buffer();
//
//      ///
//      void create_swapchain();
//
//      ///
//      void destroy_swapchain();
//
//      ///
//      void create_depth_buffer();
//
//      ///
//      void destroy_depth_buffer();
//
//      ///
//      void create_pipeline();
//
//      ///
//      void destroy_pipeline();
//
//    private:
//      ///
//      void image_layout_transition(
//        vk::image& image, vk::image_aspect_flags aspect_mask,
//        vk::image_layout old_image_layout, vk::image_layout new_image_layout,
//        std::uint32_t source_queue_family_index = 0,
//        std::uint32_t destination_queue_family_index = 0);
//
//      bool _quit = false;
//      core::stack_ptr<vk::renderer> _renderer;
//      std::unique_ptr<vk::window> _window;
//      boost::intrusive_ptr<vk::device> _device;
//
//      std::vector<swapchain_element> _swapchain_elements;
//
//      boost::intrusive_ptr<vk::command_buffer> _init_command_buffer;
//
//      boost::intrusive_ptr<vk::image> _depth_image;
//      boost::intrusive_ptr<vk::image_view> _depth_image_view;
//
//      std::uint32_t _current_buffer = 0;
//    };
//
//    void test_application::run()
//    {
//      using namespace shift;
//      using namespace shift::render;
//
//      _renderer.create(1, "test.shift.render.vk", 1,
//                       vk::debug_flag::vk_standard_validation);
//      auto destroy_renderer =
//        core::make_at_exit_scope([&]() { _renderer.destroy(); });
//
//      const auto& physical_devices = _renderer->available_physical_devices();
//      // Query fine-grained feature support for these devices. If the app has
//      // specific feature requirements it should filter here unsupported
//      // devices.
//      _window = _renderer->create_window(
//        {100, 100}, {800u, 600u},
//        vk::window_flag::resizeable | vk::window_flag::decorated,
//        *physical_devices.front());
//      _window->on_close.connect([&]() { _quit = true; });
//      _window->on_resize.connect([&](math::vector<2, std::uint32_t> size) {
//        // destroy_application_resources();
//        resize_window();
//        // create_application_resources();
//      });
//      _window->on_key.connect(
//        [&](int key, int scancode, int action, int mods) {});
//      _window->on_mouse_button.connect(
//        [&](int button, int action, int mods) {});
//      _window->visible(true);
//      _window->create_surface();
//
//      _device = _renderer->create_device(*_window);
//
//      create_resources();
//
//      using namespace std::chrono;
//      // Main message processing loop.
//      auto start = high_resolution_clock::now();
//      auto now = start;
//      auto last_fps_update = start;
//      std::size_t fps_counter = 0;
//      while (!_quit && (now - start <= 30s))
//      {
//        // Wait for work to finish before updating MVP.
//        _device->wait_idle();
//
//        auto& primary_command_buffer = _window->primary_command_buffer();
//        primary_command_buffer.begin(vk::command_buffer_begin_info{});
//        primary_command_buffer.end();
//
//        _window->present();
//        _renderer->update();
//
//        glfwPollEvents();
//
//        now = high_resolution_clock::now();
//        ++fps_counter;
//        if (now - last_fps_update >= 1s)
//        {
//          shift::log::status()
//            << (fps_counter * 1000.0f /
//                duration_cast<milliseconds>(now - last_fps_update).count())
//            << " fps";
//          fps_counter = 0;
//          last_fps_update = now;
//        }
//      }
//
//      destroy_resources();
//    }
//
//    const physical_device& test_application::select_physical_device(
//      const std::vector<physical_device>& devices)
//    {
//      return devices.front();
//    }
//
//    void test_application::create_resources()
//    {
//      _device->create_resources();
//      create_init_command_buffer();
//      _window->create_swapchain(*_device);
//      create_swapchain();
//      create_depth_buffer();
//      create_pipeline();
//    }
//
//    void test_application::destroy_resources()
//    {
//      destroy_pipeline();
//      destroy_depth_buffer();
//      destroy_swapchain();
//      _window->destroy_swapchain();
//      destroy_init_command_buffer();
//      _device->destroy_resources();
//    }
//
//    void test_application::resize_window()
//    {
//      destroy_pipeline();
//      destroy_depth_buffer();
//      destroy_swapchain();
//      _swapchain_elements.clear();  // ?
//
//      _device->collect_garbage();
//      _window->create_swapchain(*_device);
//      create_swapchain();
//      create_depth_buffer();
//      create_pipeline();
//    }
//
//    void test_application::create_init_command_buffer()
//    {
//      BOOST_ASSERT(!_init_command_buffer);
//      _init_command_buffer =
//        _device->create_command_buffer(true,
//        vk::command_buffer_level::primary);
//      _init_command_buffer->begin(vk::command_buffer_begin_info(
//        /* flags */ vk::command_buffer_usage_flag::one_time_submit_bit,
//        /* inheritance_info */ nullptr));
//    }
//
//    void test_application::destroy_init_command_buffer()
//    {
//      _init_command_buffer.reset();
//    }
//
//    void test_application::flush_init_command_buffer()
//    {
//      BOOST_ASSERT(_init_command_buffer);
//
//      _init_command_buffer->end();
//
//      VkFence null_fence = VK_NULL_HANDLE;
//
//      vk_check(vk::queue_submit(
//        _device->graphics_queue().handle(), 1,
//        core::rvalue_address(vk::submit_info(
//          /* wait_semaphore_count */ 0,
//          /* wait_semaphores */ nullptr,
//          /* wait_dst_stage_mask */ nullptr,
//          /* command_buffer_count */ 1,
//          /* command_buffers */ &_init_command_buffer->handle(),
//          /* signal_semaphore_count */ 0,
//          /* signal_semaphores */ nullptr)),
//        null_fence));
//      _device->graphics_queue().wait_idle();
//
//      destroy_init_command_buffer();
//      create_init_command_buffer();
//    }
//
//    void test_application::create_swapchain()
//    {
//      // Extract images from swapchain.
//      auto& swapchain = _window->swapchain();
//      auto swapchain_images = swapchain.images();
//      _swapchain_elements.resize(swapchain.image_count());
//      for (auto i = 0u; i < _swapchain_elements.size(); ++i)
//      {
//        _swapchain_elements[i].image =
//          _device->create_image(swapchain.image_handle(i),
//          vk::image_type::_2d,
//                                swapchain.surface_format(),
//                                vk::extent_3d{swapchain.extent().width(),
//                                              swapchain.extent().height(),
//                                              1u},
//                                1u, 1u);
//
//        // Render loop will expect image to:
//        // - have been used before
//        // - be in vk::image_layout::present_src_khr layout
//        // - will change to COLOR_ATTACHMENT_OPTIMAL layout
//        // so init the image to that state.
//        _init_command_buffer->pipeline_barrier(
//          vk::pipeline_stage_flag::top_of_pipe_bit,
//          vk::pipeline_stage_flag::bottom_of_pipe_bit, 0,
//          vk::image_memory_barrier(
//            /* src_access_mask */ vk::access_flag::none,
//            /* dst_access_mask */ vk::access_flag::memory_read_bit,
//            /* old_layout */ vk::image_layout::undefined,
//            /* new_layout */ vk::image_layout::present_src_khr,
//            /* src_queue_family_index */
//            _device->graphics_queue_family_index(),
//            /* dst_queue_family_index */
//            _device->present_queue_family_index(),
//            /* image */ _swapchain_elements[i].image->handle(),
//            /* subresource_range */
//            vk::image_subresource_range(
//              /* aspect_mask */ vk::image_aspect_flag::color_bit,
//              /* base_mip_level */ 0,
//              /* level_count */ 1,
//              /* base_array_layer */ 0,
//              /* layer_count */ 1)));
//
//        _swapchain_elements[i].image_view = _device->create_image_view(
//          vk::image_view_create_info(
//            /* flags */ 0,
//            /* image */ _swapchain_elements[i].image->handle(),
//            /* view_type */ vk::image_view_type::_2d,
//            /* format */ _window->surface().format(),
//            /* components */
//            vk::component_mapping{
//              vk::component_swizzle::r, vk::component_swizzle::g,
//              vk::component_swizzle::b, vk::component_swizzle::a},
//            /* subresource_range */
//            vk::image_subresource_range(
//              /* aspect_mask */ vk::image_aspect_flag::color_bit,
//              /* base_mip_level */ 0,
//              /* level_count */ 1,
//              /* base_array_layer */ 0,
//              /* layer_count */ 1)),
//          *_swapchain_elements[i].image);
//      }
//    }
//
//    void test_application::destroy_swapchain()
//    {
//      /// ToDo: Simply clearing the vector should be sufficient.
//      _swapchain_elements.clear();
//    }
//
//    void test_application::create_depth_buffer()
//    {
//      // const VkFormat depth_format = vk::format::d16_unorm;
//      const vk::format depth_format = vk::format::d32_sfloat;
//      _depth_image = _device->create_image(vk::image_create_info(
//        /* flags */ vk::image_create_flag::none,
//        /* image_type */ vk::image_type::_2d,
//        /* format */ depth_format,
//        /* extent */
//        vk::extent_3d{_window->size().width, _window->size().height, 1u},
//        /* mip_levels */ 1,
//        /* array_layers */ 1,
//        /* samples */ vk::sample_count_flag::_1_bit,
//        /* tiling */ vk::image_tiling::optimal,
//        /* usage */ vk::image_usage_flag::depth_stencil_attachment_bit,
//        /* sharing_mode */ vk::sharing_mode::exclusive,
//        /* queue_family_index_count */ 0,
//        /* queue_family_indices */ nullptr,
//        /* initial_layout */ vk::image_layout::undefined));
//
//      _depth_image->allocate_storage(
//        vk::memory_property_flag::device_local_bit);
//
//      image_layout_transition(
//        *_depth_image, vk::image_aspect_flag::depth_bit,
//        vk::image_layout::undefined,
//        vk::image_layout::depth_stencil_attachment_optimal);
//
//      _depth_image_view = _device->create_image_view(
//        vk::image_view_create_info(
//          /* flags */ 0,
//          /* image */ _depth_image->handle(),
//          /* view_type */ vk::image_view_type::_2d,
//          /* format */ depth_format,
//          /* components */ vk::component_mapping{},
//          /* subresource_range */
//          vk::image_subresource_range(
//            /* aspect_mask */ vk::image_aspect_flag::depth_bit,
//            /* base_mip_level */ 0,
//            /* level_count */ 1,
//            /* base_array_layer */ 0,
//            /* layer_count */ 1)),
//        *_depth_image);
//    }
//
//    void test_application::destroy_depth_buffer()
//    {
//      _depth_image_view.reset();
//      _depth_image.reset();
//    }
//
//    void test_application::create_pipeline()
//    {
//      _window->create_pipeline();
//      flush_init_command_buffer();
//      _current_buffer = 0;
//    }
//
//    void test_application::destroy_pipeline()
//    {
//      _window->destroy_pipeline();
//    }
//
//    void test_application::image_layout_transition(
//      vk::image& image, vk::image_aspect_flags aspect_mask,
//      vk::image_layout old_image_layout, vk::image_layout new_image_layout,
//      std::uint32_t source_queue_family_index,
//      std::uint32_t destination_queue_family_index)
//    {
//      VkAccessFlags dst_access_mask;
//      /// ToDo: Review this automatic flag assignment logic.
//      if (new_image_layout == vk::image_layout::transfer_dst_optimal)
//      {
//        // Make sure anything that was copying from this image has completed
//        dst_access_mask = VK_ACCESS_TRANSFER_READ_BIT;
//      }
//      else if (new_image_layout == vk::image_layout::color_attachment_optimal)
//      {
//        dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//      }
//      else if (new_image_layout ==
//               vk::image_layout::depth_stencil_attachment_optimal)
//      {
//        dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//      }
//      else if (new_image_layout == vk::image_layout::shader_read_only_optimal)
//      {
//        // Make sure any Copy or CPU writes to image are flushed
//        dst_access_mask =
//          VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
//      }
//      else
//        dst_access_mask = 0;
//
//      _init_command_buffer->pipeline_barrier(
//        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
//        0, vk::image_memory_barrier(
//          /* src_access_mask */ vk::access_flag::none,
//          /* dst_access_mask */ dst_access_mask,
//          /* old_layout */ old_image_layout,
//          /* new_layout */ new_image_layout,
//          /* src_queue_family_index */ source_queue_family_index,
//          /* dst_queue_family_index */ destination_queue_family_index,
//          /* image */ image.handle(),
//          /* subresource_range */
//          vk::image_subresource_range(
//            /* aspect_mask */ aspect_mask,
//            /* base_mip_level */ 0,
//            /* level_count */ 1,
//            /* base_array_layer */ 0,
//            /* layer_count */ 1)));
//    }
//  }
//}
//}
//
// using namespace shift;
//
// BOOST_AUTO_TEST_CASE(window_test)
//{
//  using namespace std::chrono;
//  using namespace shift::log;
//  using namespace shift::render;
//
//  log_server::create();
//  auto destroy_log_server =
//    shift::core::make_at_exit_scope([]() { log_server::destroy(); });
//  log_server::singleton_instance().add_console_sink();
//
//  shift::render::vk::test_application my_test_application;
//  my_test_application.run();
//}
