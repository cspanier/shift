#ifndef SHIFT_CORE_EXCEPTION_HPP
#define SHIFT_CORE_EXCEPTION_HPP

#include <string>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/info_tuple.hpp>
#include <boost/stacktrace/stacktrace_fwd.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::core
{
/// Boost exceptions in a nutshell:
/// - Compared to std::exceptions they provide an extendible way to attach
///   arbitrary data to the exception.
/// - Define own custom base exception type:
///   struct my_generic_error : virtual boost::exception,
///                             virtual std::exception {};
///   (don't forget to use virtual inheritance!)
/// - Define own custom specific/derived exception type:
///   struct my_specific_error : virtual my_generic_error {};
/// - Define a new information field which can be attached to any Boost
///   exception:
///   using my_info = boost::error_info<struct my_info_tag, any_type>;
///   - Replace any_type with some common type like std::string, int, float,
///     bool, ... (any type T that has an operator
///     std::ostream&<<(std::ostream&, const T&);)
///   - my_info_tag must be globally unique and does not have to be defined.
/// - Throw exceptions:
///     BOOST_THROW_EXCEPTION(my_specific_error() << my_info("Error
///     message"));
///     - The macro BOOST_THROW_EXCEPTION adds source filename and line info
///       to the exception (debug build only). Alternatively, one can use the
///       regular throw my_specific_error() << my_info("Error message");
///       syntax.
/// - Catch, add info and rethrow exceptions:
///   try { ... }
///   catch (boost::exception& e)
///   {
///     e << my_info("More info about the error");
///     throw;
///   }
/// - Catch and print exceptions:
///   try { ... }
///   catch (const boost::exception& e)
///   {
///     shift::log::exception() << boost::diagnostic_information(e);
///   }

using traced =
  boost::error_info<struct traced_tag, boost::stacktrace::stacktrace>;

#define SHIFT_THROW_EXCEPTION(x) BOOST_THROW_EXCEPTION(x)

/// Base class for almost all exceptions directly thrown by this library
/// collection.
struct error : virtual boost::exception, virtual std::exception
{
};

using context_info = boost::error_info<struct context_info_tag, std::string>;

template <typename T>
using error_code_info = boost::error_info<struct error_code_info_tag, T>;

/// Base class for exceptions caused by evaluation of run-time computed
/// information.
struct runtime_error : virtual error
{
};

/// Base class for exceptions caused by wrong usage of some type.
struct logic_error : virtual error
{
};

/// An exception type to be used when an out of memory situation occurs.
struct out_of_memory : virtual runtime_error
{
};

/// An exception type to be used when a method call is invalid for the
/// object's current state.
/// @remarks
///   Any invalid_operation should have a context_info attachment.
struct invalid_operation : virtual runtime_error
{
};

/// An exception type to be used when one of the arguments provided to a
/// method is invalid.
/// @remarks
///   Any invalid_argument should have a context_info attachment.
struct invalid_argument : virtual runtime_error
{
};

/// An exception type for any IO error
struct io_error : virtual runtime_error
{
};

using file_name_info =
  boost::error_info<struct file_name_info_tag, std::string>;

/// An exception type for any file related error
/// @remarks
///   Any file_error should have a file_name_info attachment.
struct file_error : virtual io_error
{
};

/// An exception type thrown if some kind of file could not be opened.
struct file_open_error : virtual file_error
{
};

/// An exception type thrown if some kind of file could not be read from.
struct file_read_error : virtual file_error
{
};

/// An exception type thrown if some kind of file could not be written to.
struct file_write_error : virtual file_error
{
};

using path_name_info =
  boost::error_info<struct path_name_info_tag, std::string>;

/// An exception type thrown if some path is invalid or non-existent.
struct path_error : virtual io_error
{
};
}

#endif
