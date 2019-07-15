#ifndef SHIFT_LIVEDEBUG_LAUNCHER_HPP
#define SHIFT_LIVEDEBUG_LAUNCHER_HPP

#include <cstdint>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/program_options.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/livedebug/debug_server.hpp"

namespace shift::livedebug
{
template <typename NextModule>
class launcher : public NextModule
{
public:
  using base_t = NextModule;

  launcher(int argc, char* argv[]) : base_t(argc, argv)
  {
    namespace opt = boost::program_options;

    base_t::_hidden_options.add_options()(
      "livedebug-address",
      opt::value(&_bind_address)->default_value("127.0.0.1"),
      "The IP address the HTTP server shall bind to.");
    base_t::_hidden_options.add_options()(
      "livedebug-port", opt::value(&_port)->default_value(8080),
      "The TCP port the HTTP server shall use.");
    base_t::_hidden_options.add_options()(
      "livedebug-threads", opt::value(&_thread_count)->default_value(4),
      "The number of worker threads for the HTTP server.");
  }

  ~launcher() override = default;

protected:
  void start() override
  {
    base_t::start();

    auto& server = debug_server::singleton_create(_thread_count);
    server.listen(boost::asio::ip::tcp::endpoint{
      boost::asio::ip::address::from_string(_bind_address), _port});
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

private:
  std::string _bind_address;
  std::uint16_t _port;
  std::uint16_t _thread_count;
};
}

#endif
