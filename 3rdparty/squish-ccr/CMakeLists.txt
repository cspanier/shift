shift_add_library(3rdparty.squish-ccr STATIC
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEFINITIONS
    SQUISH_USE_SSE=4
    SQUISH_USE_XSSE=0
  DEPENDENCIES
    GSL
)

shift_add_test(test.3rdparty.squish-ccr
  VERSION 1.0
  SOURCEROOTS
    ${CMAKE_CURRENT_SOURCE_DIR}/test
  DEPENDENCIES
    3rdparty.squish-ccr
    Boost::boost
    Boost::unit_test_framework
    GSL
)
