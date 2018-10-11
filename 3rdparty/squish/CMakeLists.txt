shift_add_library(3rdparty.squish STATIC
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEFINITIONS
    SQUISH_USE_SSE=2
)

shift_add_test(test.3rdparty.squish
  VERSION 1.0
  SOURCEROOTS
    ${CMAKE_CURRENT_SOURCE_DIR}/test
  DEPENDENCIES
    3rdparty.squish
    Boost::boost
    Boost::unit_test_framework
)
