#include "shift/livedebug/session.h"
#include <boost/beast/version.hpp>

namespace shift::livedebug
{
using namespace boost::beast;

session::session(debug_server& server) : _server(server)
{
}

std::shared_ptr<http::response<http::string_body>> session::bad_request(
  string_view why, unsigned int http_version, bool http_keep_alive)
{
  auto response = std::make_shared<http::response<http::string_body>>(
    http::status::bad_request, http_version);
  response->set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response->set(http::field::content_type, "text/html");
  response->keep_alive(http_keep_alive);
  response->body() = std::string{why};
  response->prepare_payload();
  return response;
};

std::shared_ptr<http::response<http::string_body>> session::not_found(
  string_view target, unsigned int http_version, bool http_keep_alive)
{
  auto response = std::make_shared<http::response<http::string_body>>(
    http::status::not_found, http_version);
  response->set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response->set(http::field::content_type, "text/html");
  response->keep_alive(http_keep_alive);
  response->body() = "The resource '" + target.to_string() + "' was not found.";
  response->prepare_payload();
  return response;
};

std::shared_ptr<http::response<http::string_body>> session::server_error(
  string_view what, unsigned int http_version, bool http_keep_alive)
{
  auto response = std::make_shared<http::response<http::string_body>>(
    http::status::internal_server_error, http_version);
  response->set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response->set(http::field::content_type, "text/html");
  response->keep_alive(http_keep_alive);
  response->body() = "An error occurred: '" + std::string{what} + "'";
  response->prepare_payload();
  return response;
};
}
