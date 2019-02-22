#ifndef SHIFT_PROTO_GEN_FILEWRITER_HPP
#define SHIFT_PROTO_GEN_FILEWRITER_HPP

#include <functional>
#include <sstream>
#include <filesystem>

namespace shift::proto
{
struct namescope;

/// A simple helper class which acts like a file stream buffer, which gets
/// written to the file on destruction, only if the file content has changed.
class file_writer
{
public:
  /// Returns a copy of the internal buffer.
  std::string str() const;

  /// Returns the global name scope object.
  const namescope& global_scope() const;

  /// Sets the global name scope object.
  void global_scope(const namescope& scope);

  /// Returns the current name scope object.
  const namescope& current_scope() const;

  /// Write the buffered stream to the specified file, but only if buffer
  /// content is different to the file content. Optionally, it can apply
  /// clang-format to the written source file.
  void write(std::filesystem::path filename,
             std::filesystem::path clang_format = "");

  /// Clears the file content buffer.
  void clear();

  template <typename T>
  inline file_writer& operator<<(const T& value)
  {
    _content << value;
    return *this;
  }

  inline file_writer& operator<<(std::ostream& (*function)(std::ostream&))
  {
    _content << function;
    return *this;
  }

  inline file_writer& operator<<(std::ios& (*function)(std::ios&))
  {
    _content << function;
    return *this;
  }

  inline file_writer& operator<<(std::ios_base& (*function)(std::ios_base&))
  {
    _content << function;
    return *this;
  }

public:
  using enter_namescope =
    std::function<void(std::ostream& stream, const namescope&)>;
  using leave_namescope =
    std::function<void(std::ostream& stream, const namescope&)>;

  /// Switches to another name scope.
  bool switch_namescope(const namescope& target);

  enter_namescope on_enter_namescope;
  leave_namescope on_leave_namescope;

private:
  std::ostringstream _content;
  const namescope* _global_scope = nullptr;
  const namescope* _current_scope = nullptr;
};
}

#endif
