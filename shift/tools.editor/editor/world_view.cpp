//#include "editor/world_view.h"
//#include "editor/editor_application.h"
//#include <shift/render/vk/renderer.h>
//#include <shift/log/log.h>
//#include <QtQuick/qquickwindow.h>
//#include <QtGui/QOpenGLShaderProgram>
//#include <QtGui/QOpenGLContext>
//#include <QtOpenGLExtensions/QtOpenGLExtensions>

// namespace shift
//{
// world_view::world_view()
//{
//  connect(this, &QQuickItem::windowChanged, this,
//          &world_view::handle_window_changed);
//  connect(this, &world_view::check_signal_semaphores, this,
//          &world_view::signal_semaphores, Qt::QueuedConnection);
//}

// world_view::~world_view()
//{
//}

// void world_view::release_image(std::uint64_t image, std::uint32_t
// image_index)
//{
//  if (!image)
//    return;

//  {
//    std::lock_guard lock(_queue_mutex);
//    _ready_queue.push({image, image_index});
//  }
//  _ready_condition.notify_one();
//}

// void world_view::present(std::uint64_t image, std::uint32_t image_index,
//                         std::uint32_t wait_semaphore_count,
//                         std::uint64_t* wait_semaphores)
//{
//  BOOST_ASSERT(image);
//  BOOST_ASSERT(wait_semaphore_count == 0 || wait_semaphores);

//  log::debug() << "present image_index #" << image_index << " semaphore #"
//               << std::hex << *wait_semaphores;

//  std::lock_guard lock(_queue_mutex);
//  _present_queue.push(
//    {image,
//     image_index,
//     {wait_semaphores, wait_semaphores + wait_semaphore_count}});
//  emit update();
//}

// std::uint32_t world_view::acquire_next_image(std::uint64_t signal_semaphore)
//{
//  log::debug() << "acquire_next_image present_complete_semaphore #" <<
//  std::hex
//               << signal_semaphore;
//  std::uint32_t next_image_index = ~0u;
//  {
//    std::unique_lock lock(_queue_mutex);
//    _ready_condition.wait(lock,
//                          [&]() { return _quit || !_ready_queue.empty(); });
//    if (_quit)
//      return ~0u;
//    BOOST_ASSERT(!_ready_queue.empty());
//    next_image_index = _ready_queue.front().image_index;
//    _ready_queue.pop();
//    _signal_semaphores.push(signal_semaphore);
//  }
//  emit check_signal_semaphores();
//  return next_image_index;
//}

// void world_view::before_synchronizing()
//{
//  if (!_parent_window)
//    return;
//  // _renderer->viewport_size(window()->size() *
//  window()->devicePixelRatio()); _viewport_position =
//  QPoint(static_cast<int>(x()), static_cast<int>(y())) *
//                       _parent_window->devicePixelRatio();
//  _viewport_size =
//    QSize(static_cast<int>(width()), static_cast<int>(height())) *
//    _parent_window->devicePixelRatio();
//}

// void world_view::scene_graph_initialized()
//{
//  if (!_parent_window)
//    qFatal("Logic error.");
//  initializeOpenGLFunctions();

//  auto* context = _parent_window->openglContext();
//  // Check if GL_NV_draw_vulkan_image extension is supported.
//  if (!context->hasExtension(QByteArrayLiteral("GL_NV_draw_vulkan_image")))
//    qFatal("GL_NV_draw_vulkan_image is not supported.");

//  glDrawVkImageNV = reinterpret_cast<glDrawVkImageNVFun>(
//    context->getProcAddress("glDrawVkImageNV"));
//  glWaitVkSemaphoreNV = reinterpret_cast<glWaitVkSemaphoreNVFun>(
//    context->getProcAddress("glWaitVkSemaphoreNV"));
//  glSignalVkSemaphoreNV = reinterpret_cast<glSignalVkSemaphoreNVFun>(
//    context->getProcAddress("glSignalVkSemaphoreNV"));
//  glSignalVkFenceNV = reinterpret_cast<glSignalVkFenceNVFun>(
//    context->getProcAddress("glSignalVkFenceNV"));
//  if (!glDrawVkImageNV || !glWaitVkSemaphoreNV || !glSignalVkSemaphoreNV ||
//      !glSignalVkFenceNV)
//  {
//    qFatal("Cannot load GL_NV_draw_vulkan_image functions.");
//  }

//  //  glGenSamplers(1, &_sampler);
//  //  glSamplerParameteri(_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//  //  glSamplerParameteri(_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

//  /// ToDo: What do to which potential leftovers?
//  _present_queue = {};
//  _ready_queue = {};
//}

// void world_view::scene_graph_invalidated()
//{
//  //  if (_sampler)
//  //  {
//  //    glDeleteSamplers(1, &_sampler);
//  //    _sampler = 0;
//  //  }
//}

// void world_view::before_rendering()
//{
//  if (!_parent_window)
//    return;
//  if (!glDrawVkImageNV)
//    qFatal("Logic error.");

//  glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
//  glScissor(_viewport_position.x(), _viewport_position.y(),
//            _viewport_size.width(), _viewport_size.height());
//  glViewport(_viewport_position.x(), _viewport_position.y(),
//             _viewport_size.width(), _viewport_size.height());

//  glDisable(GL_DEPTH_TEST);

//  glClearColor(0.5, 0.5, 0.5, 1);
//  glClear(GL_COLOR_BUFFER_BIT);

//  {
//    std::lock_guard lock(_queue_mutex);
//    // Display the latest image only and move any other images to the ready
//    // queue.
//    while (!_present_queue.empty())
//    {
//      auto& next = _present_queue.front();
//      // Acquire all wait semaphores on GL server side.
//      for (auto wait_semaphore : next.wait_semaphores)
//      {
//        log::debug() << "glWaitVkSemaphoreNV #" << std::hex << wait_semaphore;
//        glWaitVkSemaphoreNV(wait_semaphore);
//      }
//      if (_present_queue.size() == 1)
//      {
//        log::debug() << "glDrawVkImageNV #" << std::hex << next.image_index;
//        glDrawVkImageNV(next.image, 0, 0.0f, 0.0f,
//        static_cast<float>(width()),
//                        static_cast<float>(height()), 0.0f, 0.0f, 0.0f, 1.0f,
//                        1.0f);
//      }
//      _ready_queue.push({next.image, next.image_index});
//      _present_queue.pop();
//    }
//  }
//  _ready_condition.notify_one();

//  glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);

//  // Not strictly needed for this example, but generally useful for when
//  // mixing with raw OpenGL.
//  _parent_window->resetOpenGLState();
//}

// void world_view::handle_window_changed(QQuickWindow* window)
//{
//  if (_parent_window == window)
//    return;

//  /// ToDo: How to disconnect signals?
//  //  if (_parent_window)
//  //  {
//  //   // This will disconnect all signals from with the receiver's slot.
//  //    disconnect(_parent_window, SLOT(update));
//  //  }

//  if (window)
//  {
//    connect(this, &world_view::update, window, &QQuickWindow::update);
//    connect(window, &QQuickWindow::beforeSynchronizing, this,
//            &world_view::before_synchronizing, Qt::DirectConnection);
//    connect(window, &QQuickWindow::sceneGraphInitialized, this,
//            &world_view::scene_graph_initialized, Qt::DirectConnection);
//    connect(window, &QQuickWindow::sceneGraphInvalidated, this,
//            &world_view::scene_graph_invalidated, Qt::DirectConnection);
//    connect(window, &QQuickWindow::beforeRendering, this,
//            &world_view::before_rendering, Qt::DirectConnection);
//    // If we allow QML to do the clearing, they would clear what we paint and
//    // nothing would show.
//    window->setClearBeforeRendering(false);

//    _parent_window = window;
//  }
//  else
//  {
//    _parent_window = nullptr;
//    glDrawVkImageNV = nullptr;
//    glWaitVkSemaphoreNV = nullptr;
//    glSignalVkSemaphoreNV = nullptr;
//    glSignalVkFenceNV = nullptr;
//  }
//}

// void world_view::signal_semaphores()
//{
//  // Check that we're on the render thread.
//  BOOST_ASSERT(QThread::currentThread() == thread());

//  std::unique_lock lock(_queue_mutex);
//  // Signal all images' semaphores on GL server side.
//  while (!_signal_semaphores.empty())
//  {
//    auto semaphore_handle = _signal_semaphores.front();
//    log::debug() << "glSignalVkSemaphoreNV present_complete_semaphore #"
//                 << std::hex << semaphore_handle;
//    glSignalVkSemaphoreNV(semaphore_handle);
//    _signal_semaphores.pop();
//  }
//}
//}
