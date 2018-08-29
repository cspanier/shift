#define BOOST_USE_WINDOWS_H
#include "shift/platform/crash_guard.h"
#include "shift/platform/environment.h"
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <string>
#include <client/windows/handler/exception_handler.h>

namespace shift::platform
{
/// The Windows specific crash_guard implementation.
class crash_guard::impl
{
public:
  /// Constructor setting up Google Breakpad.
  impl(const std::wstring& dump_path);

  impl(const impl&) = delete;
  impl(impl&&) = delete;
  ~impl() = default;
  impl& operator=(const impl&) = delete;
  impl& operator=(impl&&) = delete;

  /// Exception filter callback used to ignore certain types of system
  /// exceptions.
  static bool exception_filter(void* context, EXCEPTION_POINTERS* exception,
                               MDRawAssertionInfo* assertion);

  /// A final callback used to report the path to the minidump file and to
  /// decide whether to continue execution or to crash the application.
  static bool minidump_callback(const wchar_t* dump_path,
                                const wchar_t* minidump_id, void* context,
                                EXCEPTION_POINTERS* exception,
                                MDRawAssertionInfo* assertion, bool succeeded);

  // The exception code for C++ exceptions.
  static constexpr DWORD EXCEPTION_ACCESS_DENIED = 0x00000005;
  static constexpr DWORD EXCEPTION_HEAP_DAMAGED = 0xC0000374;
  static constexpr DWORD EXCEPTION_RPC_SERVER_UNAVAILABLE = 0x000006BA;
  static constexpr DWORD EXCEPTION_RPC_CALL_CANCELED_OR_TIMED_OUT = 0x0000071A;
  static constexpr DWORD EXCEPTION_MSVC_OUTPUT_DEBUG_STRING = 0x40010006;
  static constexpr DWORD EXCEPTION_RENDERDOC_OUTPUT_DEBUG_STRING = 0x4001000A;
  static constexpr DWORD EXCEPTION_MSVC_DEBUG_THREAD_NAME = 0x406D1388;
  static constexpr DWORD EXCEPTION_MPX_ARRAY_BOUNDS_EXCEEDED = 0xC000008C;
  static constexpr DWORD EXCEPTION_CPP = 0xE06D7363;

  google_breakpad::ExceptionHandler exception_handler;
};

crash_guard::impl::impl(const std::wstring& dump_path)
: exception_handler(dump_path.c_str(), exception_filter, minidump_callback,
                    this, google_breakpad::ExceptionHandler::HANDLER_ALL,
                    static_cast<MINIDUMP_TYPE>(
                      MiniDumpNormal | MiniDumpWithIndirectlyReferencedMemory |
                      MiniDumpWithDataSegs),
                    L"", nullptr)
{
}

bool crash_guard::impl::exception_filter(void* /*context*/,
                                         EXCEPTION_POINTERS* exception,
                                         MDRawAssertionInfo* /*assertion*/)
{
  // auto* instance = static_cast<crash_guard::impl*>(context);
  if (exception && exception->ExceptionRecord)
  {
    switch (exception->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_CPP:
    case EXCEPTION_ACCESS_DENIED:
    case EXCEPTION_RPC_SERVER_UNAVAILABLE:
    case EXCEPTION_RPC_CALL_CANCELED_OR_TIMED_OUT:
    case EXCEPTION_MSVC_OUTPUT_DEBUG_STRING:
    case EXCEPTION_RENDERDOC_OUTPUT_DEBUG_STRING:
    case EXCEPTION_MSVC_DEBUG_THREAD_NAME:
    case EXCEPTION_MPX_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_BREAKPOINT:
    case EXCEPTION_SINGLE_STEP:
      // Eventually ignore more type of exceptions.
      return false;

    case EXCEPTION_HEAP_DAMAGED:
    default:
      return true;
    }
  }
  return true;
}

bool crash_guard::impl::minidump_callback(const wchar_t* dump_path,
                                          const wchar_t* /*minidump_id*/,
                                          void* /*context*/,
                                          EXCEPTION_POINTERS* /*exception*/,
                                          MDRawAssertionInfo* /*assertion*/,
                                          bool succeeded)
{
  wprintf(L"Dump path: %s\n", dump_path);
  return succeeded;
}

crash_guard::crash_guard(boost::filesystem::path dump_path)
{
  using namespace boost::filesystem;

  if (!exists(dump_path))
  {
    boost::system::error_code error;
    create_directories(dump_path, error);
    if (error)
      dump_path = current_path();
  }
  else if (!is_directory(dump_path))
  {
    BOOST_ASSERT(false);
    dump_path = current_path();
  }

  _impl = std::make_unique<impl>(dump_path.generic_wstring());
}

crash_guard::~crash_guard() = default;
}
