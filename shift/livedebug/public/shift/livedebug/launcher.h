#ifndef SHIFT_LIVEDEBUG_LAUNCHER_H
#define SHIFT_LIVEDEBUG_LAUNCHER_H

#include <chrono>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <shift/platform/signal_handler.h>
#include <shift/core/exception.h>
#include "shift/livedebug/debug_server.h"

namespace shift::livedebug
{
template <typename NextModule>
class launcher : public NextModule
{
public:
  using base_t = NextModule;

  launcher(int argc, char* argv[]) : base_t(argc, argv)
  {
  }

  ~launcher() override = default;

protected:
  void start() override
  {
    base_t::start();

    const auto address = boost::asio::ip::make_address("127.0.0.1");
    const std::uint16_t port = 8080;
    const auto threads = 1;

    auto& server = debug_server::singleton_create(threads);
    server.listen(boost::asio::ip::tcp::endpoint{address, port});
    server.start();
  }

  void stop() noexcept override
  {
    if (debug_server::singleton_instantiated())
    {
      debug_server::singleton_instance().stop();
      debug_server::singleton_destroy();
    }

    base_t::stop();
  }
};
}

#endif
