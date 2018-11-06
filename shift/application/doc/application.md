# shift.application

## Overview

This is a minimalistic library to initialize common components like

* the current working directory
* the crash guard handler that writes mini dumps
* signal handlers to catch `ctrl+c` or `ctrl+brk`
* the [log system](../../log/doc/log.md)
* common command line options including `--help`

The library is designed to allow additional launcher modules to be chained and integrated into the startup logic (e.g. command line options need to be registered before `--help` prints out all available options):

```c++
template <template <typename> class... Modules>
using launcher_t = typename launcher<Modules...>::type;
```

Each launcher module type passed as a template parameter to `launcher_t` must satisfy the following conditions:

* It must have a single template parameter and derive from that type.
* It must have a constructor with the following two parameters: `int argc, char* argv[]`. Those parameters must be passed to the base class's constructor.
* Command line options must be registered in the launcher's constructor body using the three available protected member variables `_visible_options`, `_hidden_options`, and `_positional_options` derived from the base class. The values of program options should be stored in either global or static variables so that other classes can get access without having an instance of the launcher object.
* It may override the public method `virtual int execute(std::function<int()> handler)`, but has to call the blocking base class's execute handler, optionally with a different function object that has to call the passed blocking handler function object and return its resulting error code (see example).
* It may override the two protected and non-blocking methods `void start()` and `void stop()` to perform initialization and shutdown logic. `start()` has to call `base_t::start()` first, and `stop()` has to call `base_t::stop()` last.
* Because each launcher calls its base class, order matters. Modules specified first are initialized last.

## Usage

```c++
#include <shift/application/launcher.hpp>
#include <shift/network/launcher.hpp>
#include <shift/service/launcher.hpp>

int main(int argc, char* argv[])
{
  using namespace shift;
  using shift::application::launcher_t;

  launcher_t<service::launcher, network::launcher> launcher(argc, argv);
  return launcher.execute([&]() -> int {
    // Everything outlined above is set up at this point.
    return EXIT_SUCCESS;
  });
}
```

## Available Launchers:

* [shift/network/launcher.h](../../network/public/shift/network/launcher.h)
* [shift/service/launcher.h](../../service/public/shift/service/launcher.h) (depends on network::launcher)
* [shift/livedebug/launcher.h](../../livedebug/public/shift/livedebug/launcher.h)
* [shift/task/launcher.h](../../task/public/shift/task/launcher.h)

## Example for own Launcher:

```c++
struct program_options
{
  static std::uint32_t my_option;
};

template <typename NextModule>
class launcher : public NextModule, public program_options
{
public:
  using base_t = NextModule;

  ///
  launcher(int argc, char* argv[]) : base_t(argc, argv)
  {
    namespace opt = boost::program_options;

    base_t::_visible_options.add_options()(
      "my-option",
      opt::value<std::uint32_t>(&my_option)->default_value(0),
      "Description of my option.");
  }

  ~launcher() override = default;

  int execute(std::function<int()> handler) override
  {
    return base_t::execute([&]() -> int {
      // You could spawn a thread that executes handler as long as you don't
      // return before execution has finished.

      // You could also perform additional initialization logic here that
      // doesn't fit in start().

      auto result = handler();

      // You could perform shutdown logic here that doesn't fit in stop().
      
      return result;
    });
  }

protected:
  void start() override
  {
    // It is mandatory that you first call this.
    base_t::start();

    // Your startup logic.
  }

  void stop() override
  {
    // Your shutdown logic.

    // It is mandatory that you last call this.
    base_t::stop();
  }
};
```
