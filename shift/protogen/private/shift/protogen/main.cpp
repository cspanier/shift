#include "shift/protogen/translator.hpp"
#include "shift/protogen/launcher.hpp"
#include <shift/application/launcher.hpp>

int main(int argc, char* argv[])
{
  using namespace shift;

  application::launcher_t<proto::launcher> launcher(argc, argv);
  return launcher.execute([&]() { return proto::translator{}.run(); });
}
