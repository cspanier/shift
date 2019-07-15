#include "shift/livedebug/listener.hpp"
#include "shift/livedebug/http_session.hpp"
#include <shift/log/log.hpp>

namespace shift::livedebug
{
std::error_code convert_error_code(boost::system::error_code error)
{
  if (error.category() == boost::system::generic_category())
    return std::error_code{error.value(), std::generic_category()};
  else
    return shift::livedebug::error_code::internal_error;
}

listener::listener(debug_server& server, boost::asio::io_context& io_context)
: _server(server), _io_context(io_context), _acceptor(io_context)
{
}

listener::~listener()
{
  close();
}

std::error_code listener::open(boost::asio::ip::tcp::endpoint endpoint)
{
  boost::system::error_code error;

  /// ToDo: Improve boost::system::error_code to std::error_code translation.

  // Open the acceptor
  _acceptor.open(endpoint.protocol(), error);
  if (error)
    return convert_error_code(error);

  // Allow address reuse
  _acceptor.set_option(boost::asio::socket_base::reuse_address(true), error);
  if (error)
    return convert_error_code(error);

  // Bind to the server address
  _acceptor.bind(endpoint, error);
  if (error)
    return convert_error_code(error);

  // Start listening for connections
  _acceptor.listen(boost::asio::socket_base::max_listen_connections, error);
  if (error)
    return convert_error_code(error);

  BOOST_ASSERT(_acceptor.is_open());
  do_accept();
  _endpoint = std::move(endpoint);
  return {};
}

void listener::close()
{
  if (_acceptor.is_open())
  {
    _endpoint = boost::asio::ip::tcp::endpoint{};
    _acceptor.close();
  }
}

const boost::asio::ip::tcp::endpoint& listener::endpoint() const
{
  return _endpoint;
}

void listener::listener::do_accept()
{
  _acceptor.async_accept(
    boost::asio::make_strand(_io_context),
    boost::beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

void listener::on_accept(boost::system::error_code error,
                         boost::asio::ip::tcp::socket socket)
{
  if (error)
    log::error() << "accept: " << error.message();
  else
  {
    // Create the session and run it.
    std::make_shared<http_session>(_server, std::move(socket))->run();
  }

  // Accept another connection.
  do_accept();
}
}
