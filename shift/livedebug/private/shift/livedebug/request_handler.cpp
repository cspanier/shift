#include "shift/livedebug/request_handler.hpp"
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/beast/version.hpp>
#include <boost/algorithm/string.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::livedebug
{
namespace http = boost::beast::http;

std::string_view mime_type(std::string_view path)
{
  using boost::iequals;

  const auto ext = [&path] {
    const auto pos = path.rfind(".");
    if (pos == std::string_view::npos)
      return std::string_view{};
    return path.substr(pos);
  }();

  if (iequals(ext, ".htm"))
    return "text/html";
  if (iequals(ext, ".html"))
    return "text/html";
  if (iequals(ext, ".php"))
    return "text/html";
  if (iequals(ext, ".css"))
    return "text/css";
  if (iequals(ext, ".txt"))
    return "text/plain";
  if (iequals(ext, ".js"))
    return "application/javascript";
  if (iequals(ext, ".json"))
    return "application/json";
  if (iequals(ext, ".xml"))
    return "application/xml";
  if (iequals(ext, ".swf"))
    return "application/x-shockwave-flash";
  if (iequals(ext, ".flv"))
    return "video/x-flv";
  if (iequals(ext, ".png"))
    return "image/png";
  if (iequals(ext, ".jpe"))
    return "image/jpeg";
  if (iequals(ext, ".jpeg"))
    return "image/jpeg";
  if (iequals(ext, ".jpg"))
    return "image/jpeg";
  if (iequals(ext, ".gif"))
    return "image/gif";
  if (iequals(ext, ".bmp"))
    return "image/bmp";
  if (iequals(ext, ".ico"))
    return "image/vnd.microsoft.icon";
  if (iequals(ext, ".tiff"))
    return "image/tiff";
  if (iequals(ext, ".tif"))
    return "image/tiff";
  if (iequals(ext, ".svg"))
    return "image/svg+xml";
  if (iequals(ext, ".svgz"))
    return "image/svg+xml";
  return "application/text";
}

file_request_handler::file_request_handler(std::filesystem::path&& path)
: _path(std::move(path))
{
}

bool file_request_handler::operator()(
  const boost::beast::http::request<boost::beast::http::string_body>& request,
  livedebug::session& session)
{
  // Build the path to the requested file
  auto path = _path / request.target().to_string();
  if (!path.has_filename())
    path /= "index.html";

  // Attempt to open the file
  boost::beast::error_code error;
  http::file_body::value_type body;
  body.open(path.string().c_str(), boost::beast::file_mode::scan, error);

  // Handle the case where the file doesn't exist.
  if (error == boost::system::errc::no_such_file_or_directory)
  {
    session.send(livedebug::session::not_found(
      request.target(), request.version(), request.keep_alive()));
    return true;
  }

  // Handle an unknown error.
  if (error)
  {
    session.send(livedebug::session::server_error(
      error.message(), request.version(), request.keep_alive()));
    return true;
  }

  switch (request.method())
  {
  case http::verb::head:
  {
    // Respond to HEAD request
    auto response = std::make_shared<http::response<http::empty_body>>(
      http::status::ok, request.version());
    response->set(http::field::server, BOOST_BEAST_VERSION_STRING);
    response->set(http::field::content_type,
                  mime_type(path.extension().generic_string()));
    response->content_length(body.size());
    response->keep_alive(request.keep_alive());
    session.send(std::move(response));
    return true;
  }

  case http::verb::get:
  {
    // Respond to GET request
    auto content_length = body.size();
    auto response = std::make_shared<http::response<http::file_body>>(
      std::piecewise_construct, std::make_tuple(std::move(body)),
      std::make_tuple(http::status::ok, request.version()));
    response->set(http::field::server, BOOST_BEAST_VERSION_STRING);
    response->set(http::field::content_type,
                  mime_type(path.extension().generic_string()));
    response->content_length(content_length);
    response->keep_alive(request.keep_alive());
    session.send(std::move(response));
    return true;
  }

  default:
    return false;
  }
}
}
