#include "shift/protogen/translator.h"
#include "shift/protogen/launcher.h"
#include <shift/application/launcher.h>

int main(int argc, char* argv[])
{
  using namespace shift;

  application::launcher_t<proto::launcher> launcher(argc, argv);
  return launcher.execute([&]() { return proto::translator{}.run(); });
}
