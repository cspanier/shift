//#define BOOST_BIND_NO_PLACEHOLDERS
//#include "editor/render_thread.h"
//#include <shift/render/vk/renderer.h>
//#include <shift/resource/repository.h>
//#include <shift/resource/resource_group.h>
//#include <shift/resource/scene.h>
//#include <shift/log/log.h>
//#include <shift/core/exception.h>
//#include <shift/core/at_exit_scope.h>
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
