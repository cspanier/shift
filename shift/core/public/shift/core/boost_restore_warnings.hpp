/// @see shift/core/boost_restore_warnings.hpp
#include <boost/version.hpp>
#if (BOOST_VERSION <= 106800)
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(push)
#endif
#endif
