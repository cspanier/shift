#include "shift/tools/protogen/translator.h"
#include "shift/tools/protogen/launcher.h"
#include <shift/application/launcher.h>

int main(int argc, char* argv[])
{
  using namespace shift;

  application::launcher_t<tools::protogen::launcher> launcher(argc, argv);
  return launcher.execute(
    [&]() { return tools::protogen::translator{}.run(); });
}
