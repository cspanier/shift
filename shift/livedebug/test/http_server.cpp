#include <shift/livedebug/debug_server.hpp>
#include <shift/log/log_server.hpp>
#include <shift/platform/environment.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <thread>
#include <chrono>
#include <iostream>

using namespace shift;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

BOOST_AUTO_TEST_CASE(livedebug_http_server)
{
  auto& log_server = log::log_server::singleton_create();
  log_server.add_console_sink();

  auto& server = livedebug::debug_server::singleton_create(
    2, platform::environment::executable_path().parent_path().parent_path() /
         "www");
  auto error = server.listen(boost::asio::ip::tcp::endpoint{
    boost::asio::ip::address::from_string("127.0.0.1"), 8080});
  BOOST_CHECK(!error);
  server.start();

  std::this_thread::sleep_for(1min);

  server.stop();
  livedebug::debug_server::singleton_destroy();

  log::log_server::singleton_destroy();
}
