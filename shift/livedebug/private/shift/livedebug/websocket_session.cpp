#include "shift/livedebug/websocket_session.h"
//#include <boost/algorithm/string.hpp>
#include <shift/log/log.h>

namespace shift::livedebug
{
namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;
using namespace std::placeholders;

websocket_session::websocket_session(debug_server& server, tcp::socket socket)
: session(server),
  _websocket(std::move(socket)),
  _strand(_websocket.get_executor()),
  _timer(_websocket.get_executor().context(),
         (std::chrono::steady_clock::time_point::max)())
{
}

void websocket_session::send(
  std::shared_ptr<http::response<http::empty_body>>&& /*message*/)
{
}

void websocket_session::send(
  std::shared_ptr<http::response<http::string_body>>&& /*message*/)
{
}

void websocket_session::send(
  std::shared_ptr<http::response<http::file_body>>&& /*message*/)
{
}

// Called when the timer expires.
void websocket_session::on_timer(boost::system::error_code error)
{
  if (error && error != boost::asio::error::operation_aborted)
  {
    log::error() << "timer error: " << error.message();
    return;
  }

  // Verify that the timer really expired since the deadline may have moved.
  if (_timer.expiry() <= std::chrono::steady_clock::now())
  {
    // Closing the socket cancels all outstanding operations. They
    // will complete with boost::asio::error::operation_aborted
    _websocket.next_layer().shutdown(tcp::socket::shutdown_both, error);
    _websocket.next_layer().close(error);
    return;
  }

  // Wait on the timer
  _timer.async_wait(boost::asio::bind_executor(
    _strand, std::bind(&websocket_session::on_timer, shared_from_this(), _1)));
}

void websocket_session::on_accept(boost::system::error_code error)
{
  // Happens when the timer closes the socket
  if (error == boost::asio::error::operation_aborted)
    return;

  if (error)
  {
    log::error() << "accept error: " << error.message();
    return;
  }

  // Read a message
  do_read();
}

void websocket_session::do_read()
{
  // Set the timer
  _timer.expires_after(std::chrono::seconds(15));

  // Read a message into our buffer
  _websocket.async_read(
    _buffer,
    boost::asio::bind_executor(_strand, std::bind(&websocket_session::on_read,
                                                  shared_from_this(), _1, _2)));
}

void websocket_session::on_read(boost::system::error_code error,
                                std::size_t bytes_transferred)
{
  boost::ignore_unused(bytes_transferred);

  // Happens when the timer closes the socket
  if (error == boost::asio::error::operation_aborted)
    return;

  // This indicates that the websocket_session was closed
  if (error == boost::beast::websocket::error::closed)
    return;

  if (error)
  {
    log::error() << "read error: " << error.message();
    return;
  }

  // Echo the message
  _websocket.text(_websocket.got_text());
  _websocket.async_write(
    _buffer.data(),
    boost::asio::bind_executor(_strand, std::bind(&websocket_session::on_write,
                                                  shared_from_this(), _1, _2)));
}

void websocket_session::on_write(boost::system::error_code error,
                                 std::size_t bytes_transferred)
{
  boost::ignore_unused(bytes_transferred);

  // Happens when the timer closes the socket
  if (error == boost::asio::error::operation_aborted)
    return;

  if (error)
  {
    log::error() << "write error: " << error.message();
    return;
  }

  // Clear the buffer
  _buffer.consume(_buffer.size());

  // Do another read
  do_read();
}
}
