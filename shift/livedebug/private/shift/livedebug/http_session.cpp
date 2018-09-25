#include "shift/livedebug/http_session.h"
#include "shift/livedebug/websocket_session.h"
#include "shift/livedebug/debug_server.h"
#include <boost/asio/bind_executor.hpp>
#include <shift/log/log.h>

namespace shift::livedebug
{
namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;
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

http_session::http_session(debug_server& server, tcp::socket socket)
: session(server), _socket(std::move(socket)), _strand(_socket.get_executor())
{
}

void http_session::run()
{
  do_read();
}

void http_session::do_read()
{
  // Read a request
  http::async_read(
    _socket, _buffer, _request,
    boost::asio::bind_executor(
      _strand, std::bind(&http_session::on_read, shared_from_this(), _1, _2)));
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

  // See if it is a WebSocket upgrade.
  if (boost::beast::websocket::is_upgrade(_request))
  {
    // Create a WebSocket websocket_session by transferring the socket
    return std::make_shared<websocket_session>(_server, std::move(_socket))
      ->run(std::move(_request));
  }

  // Send the response
  _server.handle_request(std::move(_request), *this);
  _request.clear();
}

///
void http_session::on_write(boost::system::error_code error,
                            std::size_t bytes_transferred,
                            std::shared_ptr<void> /*message*/, bool close)
{
  boost::ignore_unused(bytes_transferred);

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

  // Read another request.
  do_read();
}

void http_session::do_close()
{
  // Send a TCP shutdown.
  boost::system::error_code error;
  _socket.shutdown(tcp::socket::shutdown_send, error);

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

template <class Body, class Allocator>
void http_session::do_send(
  std::shared_ptr<boost::beast::http::response<
    Body, boost::beast::http::basic_fields<Allocator>>>&& message)
{
  http::async_write(
    _socket, *message,
    boost::asio::bind_executor(
      _strand, std::bind(&http_session::on_write, shared_from_this(), _1, _2,
                         message, message->need_eof())));
}
}
