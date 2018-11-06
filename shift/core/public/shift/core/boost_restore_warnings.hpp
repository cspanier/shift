// Microsoft Visual C++ generates tons of warnings with warning level 4 in
// external Boost libraries. This header restores warnings.
//
// Use this header in conjunction with boost_restore_warnings.h:
//
// #include <shift/core/boost_disable_warnings.hpp>
// #include <boost/...>
// #include <shift/core/boost_restore_warnings.hpp>
#if defined(_MSC_VER)
#pragma warning(push)
#endif
