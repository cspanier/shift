#include "shift/livedebug/debug_server.hpp"
#include "shift/livedebug/file_request_handler.hpp"
#include "shift/livedebug/listener.hpp"
#include "shift/livedebug/uri.hpp"
#include <algorithm>

#include <iostream>

namespace shift::livedebug
{
namespace http = boost::beast::http;

debug_server::debug_server(std::size_t worker_thread_count,
                           std::filesystem::path&& root_path)
: _io_context(static_cast<int>(worker_thread_count))
{
  _workers.reserve(worker_thread_count);
  add_request_handler(
    std::make_unique<file_request_handler>(std::move(root_path)));
}

debug_server::~debug_server() = default;

/// Runs the I/O service on the requested number of threads.
void debug_server::start()
{
  BOOST_ASSERT(_workers.empty());

  for (auto i = _workers.capacity(); i > 0; --i)
    _workers.emplace_back([&] { _io_context.run(); });
}

///
void debug_server::stop()
{
  for (auto& listener : _listeners)
    listener->close();
  _listeners.clear();

  _io_context.stop();
  for (auto& worker : _workers)
  {
    if (worker.joinable())
      worker.join();
  }
  _workers.clear();
}

/// Opens a new listening socket that accepts connections.
std::error_code debug_server::listen(
  const boost::asio::ip::tcp::endpoint& endpoint)
{
  for (const auto& other_listener : _listeners)
  {
    if (other_listener->endpoint() == endpoint)
      return error_code::endpoint_already_in_use;
  }
  auto& new_listener =
    _listeners.emplace_back(std::make_shared<listener>(*this, _io_context));
  if (auto error = new_listener->open(endpoint); error)
  {
    _listeners.pop_back();
    return error;
  }
  return {};
}

void debug_server::close(const boost::asio::ip::tcp::endpoint& endpoint)
{
  _listeners.erase(std::remove_if(_listeners.begin(), _listeners.end(),
                                  [endpoint](const auto& listener) {
                                    return listener->endpoint() == endpoint;
                                  }),
                   _listeners.end());
}

void debug_server::add_request_handler(
  std::unique_ptr<request_handler>&& new_handler)
{
  std::unique_lock lock(_request_handler_mutex);
  _request_handlers.emplace_back(std::move(new_handler));
}

void debug_server::handle_request(
  boost::beast::http::request<boost::beast::http::string_body>&& request,
  livedebug::session& session)
{
  // Make sure we can handle the method
  if (request.method() != http::verb::get &&
      request.method() != http::verb::head)
  {
    return session.send(livedebug::session::bad_request(
      "Unsupported HTTP-method", request.version(), request.keep_alive()));
  }

  auto to_string_view = [](const auto boost_view) -> std::string_view {
    return {boost_view.begin(), boost_view.length()};
  };

  uri_t uri;
  if (!uri_split(to_string_view(request.target()), uri))
  {
    return session.send(livedebug::session::bad_request(
      "Illegal request-URI", request.version(), request.keep_alive()));
  }
  std::cout << "HTTP request '" << request.target() << "' path:" << uri.path
            << " query:" << uri.query << " fragment:" << uri.fragment
            << std::endl;

  // Request path must be absolute and not contain "..".
  if (request.target().empty() || request.target()[0] != '/' ||
      request.target().find("..") != boost::beast::string_view::npos)
  {
    return session.send(livedebug::session::bad_request(
      "Illegal request-target", request.version(), request.keep_alive()));
  }

  std::shared_lock lock(_request_handler_mutex);
  for (const auto& request_handler : _request_handlers)
  {
    if ((*request_handler)(request, uri, session))
      return;
  }
  session.send(livedebug::session::not_found(
    request.target(), request.version(), request.keep_alive()));
}
}
