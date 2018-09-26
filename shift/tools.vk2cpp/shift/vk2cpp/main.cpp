#include "shift/vk2cpp/application.h"
#include "shift/application/launcher.h"
#include <iostream>

int main(int argc, char* argv[])
{
  using namespace shift;

  application::launcher_t<vk2cpp::launcher> launcher(argc, argv);
  return launcher.execute([]() { return vk2cpp::application{}.run(); });
}
