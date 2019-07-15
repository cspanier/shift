#ifndef SHIFT_LIVEDEBUG_DEBUG_SERVER_HPP
#define SHIFT_LIVEDEBUG_DEBUG_SERVER_HPP

#include <vector>
#include <thread>
#include <shared_mutex>
#include <filesystem>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/core/singleton.hpp>
#include "shift/livedebug/error_code.hpp"
#include "shift/livedebug/request_handler.hpp"

namespace shift::livedebug
{
class listener;

///
class debug_server final
: public core::singleton<debug_server, core::create::using_new>
{
public:
  /// Constructor.
  debug_server(std::size_t worker_thread_count,
               std::filesystem::path&& root_path);

  /// Destructor.
  ~debug_server();

  /// Runs the I/O context on the requested number of threads.
  void start();

  /// Closes all listening ports, stops the I/O context and ends all worker
  /// threads.
  void stop();

  /// Opens a new listening socket that accepts connections.
  /// @pre
  ///   The endpoint must not already be used, neither from prior calls to
  ///   listen nor by other applications.
  [[nodiscard]] std::error_code listen(
    const boost::asio::ip::tcp::endpoint& endpoint);

  /// Closes a listening socket on the specified endpoint.
  /// @pre
  ///   The server must listen on the specified endpoint.
  void close(const boost::asio::ip::tcp::endpoint& endpoint);

  /// ToDo: Add URL filter parameter.
  void add_request_handler(std::unique_ptr<request_handler>&& new_handler);

  /// This function produces an HTTP response for the given request.
  void handle_request(
    boost::beast::http::request<boost::beast::http::string_body>&& request,
    livedebug::session& session);

private:
  /// ToDo: Hide implementation details.
  boost::asio::io_context _io_context;
  std::vector<std::thread> _workers;
  std::vector<std::shared_ptr<listener>> _listeners;

  std::shared_mutex _request_handler_mutex;
  std::vector<std::unique_ptr<request_handler>> _request_handlers;
};
}

#endif
