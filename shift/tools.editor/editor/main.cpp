#include "editor/editor_application.h"
#include <shift/task/launcher.h>
#include <shift/task/async.h>
#include <shift/application/launcher.h>

int main(int argc, char* argv[])
{
  using namespace shift;
  using shift::application::launcher_t;

  launcher_t<> launcher(argc, argv);
  return launcher.execute([&]() -> int {
    //    // Bind the main task to the first worker thread as required by GLFW.
    //    return task::async(task::task_create_info{}.worker_affinity(1u),
    //                       [](int argc, char** argv) {
    // Disable all floating-point exceptions in this thread
    // as QT generates too many of them.
    platform::floating_point_exceptions fpexceptions(0);

    // editor_application::setAttribute(Qt::AA_EnableHighDpiScaling);
    argc = 1;
    return editor_application{argc, argv}.run();
    //                       },
    //                       1, argv)
    //      .get();
  });
}
