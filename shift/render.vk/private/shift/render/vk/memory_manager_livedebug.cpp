#include "shift/render/vk/memory_manager_livedebug.h"
#include <boost/beast/version.hpp>

namespace shift::render::vk
{
namespace http = boost::beast::http;

memory_manager_request_handler::memory_manager_request_handler(
  std::string&& target)
: _target(std::move(target))
{
}

bool memory_manager_request_handler::operator()(
  const boost::beast::http::request<boost::beast::http::string_body>& request,
  livedebug::session& session)
{
  if (request.target() != _target)
    return false;

  std::stringstream body;
  body << R"(<!DOCTYPE html>
<html>
  <head>
    <title>Vulkan Memory Manager</title>
  </head>
  <body>
  </body>
</html>)";

  switch (request.method())
  {
  case http::verb::head:
  {
    // Respond to HEAD request
    auto response = std::make_shared<http::response<http::empty_body>>(
      http::status::ok, request.version());
    response->set(http::field::server, BOOST_BEAST_VERSION_STRING);
    response->set(http::field::content_type, "text/html");
    response->content_length(body.str().length());
    response->keep_alive(request.keep_alive());
    response->prepare_payload();
    session.send(std::move(response));
    return true;
  }

  case http::verb::get:
  {
    // Respond to GET request
    auto response = std::make_shared<http::response<http::string_body>>(
      http::status::ok, request.version());
    response->set(http::field::server, BOOST_BEAST_VERSION_STRING);
    response->set(http::field::content_type, "text/html");
    response->content_length(body.str().length());
    response->keep_alive(request.keep_alive());
    response->body() = body.str();
    response->prepare_payload();
    session.send(std::move(response));
    return true;
  }

  default:
    return false;
  }
}
}
