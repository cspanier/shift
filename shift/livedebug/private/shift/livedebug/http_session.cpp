#include "shift/livedebug/http_session.hpp"
#include "shift/livedebug/websocket_session.hpp"
#include "shift/livedebug/debug_server.hpp"
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/asio/bind_executor.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/log/log.hpp>

namespace shift::livedebug
{
namespace http = boost::beast::http;
using namespace std::placeholders;

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(std::string_view base, boost::beast::string_view path)
{
  if (base.empty())
    return path.to_string();
  std::string result{base};
  char constexpr path_separator = '/';
  if (result.back() == path_separator)
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());
  return result;
}

http_session::http_session(debug_server& server,
                           boost::asio::ip::tcp::socket&& socket)
: session(server), _stream(std::move(socket))
{
}

void http_session::run()
{
  do_read();
}

void http_session::do_read()
{
  // Make the request empty before reading,
  // otherwise the operation behavior is undefined.
  _request = {};

  // Set the timeout.
  _stream.expires_after(std::chrono::seconds(30));

  // Read a request
  http::async_read(_stream, _buffer, _request,
                   boost::beast::bind_front_handler(&http_session::on_read,
                                                    shared_from_this()));
}

void http_session::on_read(boost::system::error_code error,
                           std::size_t /*bytes_transferred*/)
{
  //  // Happens when the timer closes the socket
  //  if (error == boost::asio::error::operation_aborted)
  //    return;

  // This means they closed the connection.
  if (error == http::error::end_of_stream)
    return do_close();

  if (error)
  {
    log::error() << "read error: " << error.message();
    return;
  }

  //  // See if it is a WebSocket upgrade.
  //  if (boost::beast::websocket::is_upgrade(_request))
  //  {
  //    // Create a WebSocket websocket_session by transferring the socket.
  //    return std::make_shared<websocket_session>(_server, std::move(_socket))
  //      ->run(std::move(_request));
  //  }

  // Send the response
  _server.handle_request(std::move(_request), *this);
}

///
void http_session::on_write(bool close, std::shared_ptr<void> message,
                            boost::system::error_code error,
                            std::size_t /*bytes_transferred*/)
{
  if (error)
  {
    log::error() << "write error: " << error.message();
    return;
  }

  if (close)
  {
    // This means we should close the connection, usually because the response
    // indicated the "Connection: close" semantic.
    return do_close();
  }

  // We're done with the response so delete it
  _response.reset();

  // Read another request.
  do_read();
}

void http_session::do_close()
{
  // Send a TCP shutdown.
  boost::beast::error_code error;
  _stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, error);

  // At this point the connection is closed gracefully.
}

void http_session::send(
  std::shared_ptr<http::response<http::empty_body>>&& message)
{
  do_send(std::move(message));
}

void http_session::send(
  std::shared_ptr<http::response<http::string_body>>&& message)
{
  do_send(std::move(message));
}

void http_session::send(
  std::shared_ptr<http::response<http::file_body>>&& message)
{

  do_send(std::move(message));
}

template <bool IsRequest, class Body, class Fields>
void http_session::do_send(
  std::shared_ptr<http::message<IsRequest, Body, Fields>>&& message)
{
  auto* message_ptr = message.get();
  http::async_write(_stream, *message_ptr,
                    boost::beast::bind_front_handler(
                      &http_session::on_write, shared_from_this(),
                      message_ptr->need_eof(), std::move(message)));
}
}
