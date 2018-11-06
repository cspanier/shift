#include "shift/service/detail/service_impl.hpp"
#include "shift/service/detail/guid_generator.hpp"
#include "shift/service/basic_service.hpp"
#include "shift/service/message_envelope.hpp"
#include <shift/serialization/compact/inputarchive.hpp>

namespace shift::service::detail
{
service_impl::service_impl(basic_service& service)
: service(service),
  service_guid(generate_guid()),
  envelope(std::make_unique<message_envelope>())
{
}

service_impl::~service_impl() = default;

std::uint16_t service_impl::specific_service_port(
  boost::asio::ip::address /*bind_address*/) const
{
  return service_port;
}

bool service_impl::add_service_provider(
  guid_t /*remote_host_guid*/, guid_t /*remote_service_guid*/,
  interface_uids_t /*remote_interface_uids*/,
  service_tag_t /*remote_service_tag*/,
  boost::asio::ip::address /*local_address*/,
  boost::asio::ip::tcp::endpoint /*remote_endpoint*/)
{
  return false;
}

void service_impl::read_messages(
  std::vector<char> buffer, std::vector<char>& read_buffer,
  serialization::protocol_version_t protocol_version, bool stream)
{
  using std::swap;
  serialization::compact_input_archive<> archive(protocol_version);
  if (!stream || read_buffer.empty())
    swap(read_buffer, buffer);
  else
  {
    // There is still data left from the last call, so append the new
    // buffer.
    auto remaining_bytes = read_buffer.size();
    read_buffer.resize(remaining_bytes +
                       static_cast<std::streamoff>(buffer.size()));
    std::copy(buffer.begin(), buffer.end(),
              read_buffer.begin() + remaining_bytes);
    buffer.clear();
  }
  archive.push(
    boost::iostreams::array_source(read_buffer.data(), read_buffer.size()));
  std::size_t next_message_pos = 0;
  try
  {
    // The eof flag is set only after an attempt to read past the end of the
    // stream. By also checking whether the next message begins at the end
    // of the buffer we save ourself an exception.
    while (!archive.eof() &&
           next_message_pos < static_cast<std::size_t>(read_buffer.size()))
    {
      serialization::message_uid_t uid;
      while (!envelope->begin_read(archive, uid))
      {
        if (archive.eof())
          break;
        archive.seekg(++next_message_pos);
      }
      if (archive.eof())
        break;
      service.read_message(archive, uid);
      next_message_pos = archive.tellg();
      envelope->end_read(archive, uid);
      next_message_pos = archive.tellg();
    }
  }
  catch (std::ios_base::failure&)
  {
    // NOP.
  }
  catch (boost::exception& e)
  {
    log::exception() << boost::diagnostic_information(e);
  }
  catch (std::exception& e)
  {
    log::exception() << "Caught unhandled standard exception: " << e.what();
  }
  catch (...)
  {
    log::exception() << "Caught unhandled exception of unknown type.";
  }

  if (stream && next_message_pos < static_cast<std::size_t>(read_buffer.size()))
  {
    // There is still data left in the buffer, so save it for the next call.
    std::copy(read_buffer.begin() + next_message_pos, read_buffer.end(),
              read_buffer.begin());
    read_buffer.resize(read_buffer.size() - next_message_pos);
  }
  else
  {
    // Discard any remaining data.
    read_buffer.clear();
  }
}
}
