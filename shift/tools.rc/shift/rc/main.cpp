#include "shift/rc/application.hpp"
#include "shift/rc/launcher.hpp"
#include <shift/application/launcher.hpp>
#include <shift/task/launcher.hpp>

int main(int argc, char* argv[])
{
  using namespace shift;

  application::launcher_t<task::launcher, rc::launcher> launcher(argc, argv);
  return launcher.execute([]() { return rc::application{}.run(); });
}
