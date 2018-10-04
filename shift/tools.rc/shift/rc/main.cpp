#include "shift/rc/application.h"
#include "shift/rc/launcher.h"
#include <shift/application/launcher.h>
#include <shift/task/launcher.h>

int main(int argc, char* argv[])
{
  using namespace shift;

  application::launcher_t<task::launcher, rc::launcher> launcher(argc, argv);
  return launcher.execute([]() { return rc::application{}.run(); });
}
