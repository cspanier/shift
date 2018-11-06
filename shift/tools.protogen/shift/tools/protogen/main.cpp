#include "shift/tools/protogen/translator.hpp"
#include "shift/tools/protogen/launcher.hpp"
#include <shift/application/launcher.hpp>

int main(int argc, char* argv[])
{
  using namespace shift;

  application::launcher_t<tools::protogen::launcher> launcher(argc, argv);
  return launcher.execute(
    [&]() { return tools::protogen::translator{}.run(); });
}
