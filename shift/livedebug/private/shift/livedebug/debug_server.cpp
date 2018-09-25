#include "shift/livedebug/debug_server.h"
#include "shift/livedebug/listener_impl.h"

namespace shift::livedebug
{
namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;

debug_server::debug_server(std::size_t worker_thread_count)
: _io_context(static_cast<int>(worker_thread_count))
{
  _workers.reserve(worker_thread_count);
}

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
  _io_context.stop();
  for (auto& worker : _workers)
  {
    if (worker.joinable())
      worker.join();
  }
}

/// Opens a new listening socket that accepts connections.
std::shared_ptr<listener> debug_server::listen(const tcp::endpoint& endpoint)
{
  auto listener = std::make_shared<listener_impl>(*this, _io_context, endpoint);
  listener->run();
  return listener;
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
    return session.send(session.bad_request(
      "Unknown HTTP-method", request.version(), request.keep_alive()));
  }

  // Request path must be absolute and not contain "..".
  if (request.target().empty() || request.target()[0] != '/' ||
      request.target().find("..") != boost::beast::string_view::npos)
  {
    return session.send(session.bad_request(
      "Illegal request-target", request.version(), request.keep_alive()));
  }

  std::shared_lock lock(_request_handler_mutex);
  for (const auto& request_handler : _request_handlers)
  {
    if ((*request_handler)(request, session))
      return;
  }
  session.send(session.not_found(request.target(), request.version(),
                                 request.keep_alive()));
}
}
