//#ifndef SHIFT_WORLD_VIEW_H
//#define SHIFT_WORLD_VIEW_H

//#include <memory>
//#include <queue>
//#include <mutex>
//#include <condition_variable>
//#include <QtQuick/QQuickItem>
//#include <QtGui/QOpenGLShaderProgram>
//#include <QtGui/QOpenGLFunctions_4_5_Core>
//#include <shift/render/vk/presentation_engine.h>

// namespace shift::render::vk
//{
// class renderer;
//}

// namespace shift
//{
// class world_view : public QQuickItem,
//                   public shift::render::vk::presentation_engine,
//                   protected QOpenGLFunctions_4_5_Core
//{
//  Q_OBJECT

// public:
//  /// Default constructor.
//  world_view();

//  /// Destructor.
//  virtual ~world_view() override;

//  ///
//  void initialize(render::vk::renderer* renderer);

//  ///
//  void finalize();

//  ///
//  virtual void release_image(std::uint64_t image,
//                             std::uint32_t image_index) override;

//  ///
//  virtual void present(std::uint64_t image, std::uint32_t image_index,
//                       std::uint32_t wait_semaphore_count,
//                       std::uint64_t* wait_semaphores) override;

//  ///
//  virtual std::uint32_t acquire_next_image(
//    std::uint64_t signal_semaphore) override;

// public slots:
//  ///
//  void before_synchronizing();

//  ///
//  void scene_graph_initialized();

//  ///
//  void scene_graph_invalidated();

//  ///
//  void before_rendering();

// private slots:
//  ///
//  void handle_window_changed(QQuickWindow* window);

//  ///
//  void signal_semaphores();

// signals:
//  void update();

//  void check_signal_semaphores();

// private:
//  struct present_element
//  {
//    std::uint64_t image;
//    std::uint32_t image_index;
//    std::vector<std::uint64_t> wait_semaphores;
//  };

//  struct ready_element
//  {
//    std::uint64_t image;
//    std::uint32_t image_index;
//  };

//  bool _quit = false;
//  QQuickWindow* _parent_window = nullptr;
//  QPoint _viewport_position;
//  QSize _viewport_size;
//  GLuint _sampler = 0;

//  std::mutex _queue_mutex;
//  std::queue<present_element> _present_queue;
//  std::condition_variable _ready_condition;
//  std::queue<ready_element> _ready_queue;
//  std::queue<std::uint64_t> _signal_semaphores;

// private:
//  using glDrawVkImageNVFun = void (*)(std::uint64_t image, GLuint sampler,
//                                      GLfloat x0, GLfloat y0, GLfloat x1,
//                                      GLfloat y1, GLfloat z, GLfloat s0,
//                                      GLfloat t0, GLfloat s1, GLfloat t1);
//  using glWaitVkSemaphoreNVFun = void (*)(std::uint64_t vkSemaphore);
//  using glSignalVkSemaphoreNVFun = void (*)(std::uint64_t vkSemaphore);
//  using glSignalVkFenceNVFun = void (*)(std::uint64_t vkFence);

//  glDrawVkImageNVFun glDrawVkImageNV = nullptr;
//  glWaitVkSemaphoreNVFun glWaitVkSemaphoreNV = nullptr;
//  glSignalVkSemaphoreNVFun glSignalVkSemaphoreNV = nullptr;
//  glSignalVkFenceNVFun glSignalVkFenceNV = nullptr;
//};
//}

//#endif
