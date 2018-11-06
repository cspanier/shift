//#define BOOST_BIND_NO_PLACEHOLDERS
//#include "editor/render_thread.h"
//#include <shift/render/vk/renderer.hpp>
//#include <shift/resource/repository.hpp>
//#include <shift/resource/resource_group.hpp>
//#include <shift/resource/scene.hpp>
//#include <shift/log/log.hpp>
//#include <shift/core/exception.hpp>
//#include <shift/core/at_exit_scope.hpp>
//#include <boost/filesystem/path.hpp>
//#include <GLFW/glfw3.h>

// namespace shift
//{
// render_thread::render_thread() : _quit(false)
//{
//}

// render_thread::~render_thread()
//{
//}

// void render_thread::start()
//{
//  _quit = false;
//  _thread = std::thread([&]() {
//    log::debug() << "loading...";
//    initialize();
//    log::debug() << "entering main loop...";
//    while (!_quit)
//    {
//      update();
//    }
//    log::debug() << "shutting down...";
//    finalize();
//  });
//}

// void render_thread::stop()
//{
//  _quit = true;
//  if (_thread.joinable())
//    _thread.join();
//}
//}
