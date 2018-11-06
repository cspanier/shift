#include "shift/livedebug/listener_impl.hpp"
#include "shift/livedebug/http_session.hpp"
#include <shift/log/log.hpp>

namespace shift::livedebug
{
using tcp = boost::asio::ip::tcp;

listener_impl::listener_impl(debug_server& server,
                             boost::asio::io_context& io_context,
                             tcp::endpoint endpoint)
: _server(server), _acceptor(io_context), _socket(io_context)
{
  boost::system::error_code error;

  // Open the acceptor
  _acceptor.open(endpoint.protocol(), error);
  if (error)
  {
    log::error() << "open listening socket: " << error.message();
    return;
  }

  // Bind to the server address
  _acceptor.bind(endpoint, error);
  if (error)
  {
    log::error() << "bind listening socket: " << error.message();
    return;
  }

  // Start listening for connections
  _acceptor.listen(boost::asio::socket_base::max_listen_connections, error);
  if (error)
  {
    log::error() << "listen: " << error.message();
    return;
  }
}

listener_impl::~listener_impl() = default;

void listener_impl::run()
{
  if (!_acceptor.is_open())
    return;
  do_accept();
}

void listener_impl::close()
{
  /// ToDo: Do we have to close the socket, the acceptor, or both?
  // _socket.close();
  _acceptor.close();
}

void listener_impl::listener_impl::do_accept()
{
  _acceptor.async_accept(
    _socket, std::bind(&listener_impl::on_accept, shared_from_this(),
                       std::placeholders::_1));
}

void listener_impl::on_accept(boost::system::error_code error)
{
  if (error)
    log::error() << "accept: " << error.message();
  else
  {
    // Create the session and run it
    std::make_shared<http_session>(_server, std::move(_socket))->run();
  }

  // Accept another connection
  do_accept();
}
}
