#include "shift/vk2cpp/application.hpp"
#include "shift/application/launcher.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
  using namespace shift;

  application::launcher_t<vk2cpp::launcher> launcher(argc, argv);
  return launcher.execute([]() { return vk2cpp::application{}.run(); });
}
