shift_add_library(shift.math STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  INCLUDEDIRS
    ${CMAKE_SOURCE_DIR}/shift/serialization/public
  DEPENDENCIES
    shift.core
    shift.platform
    Boost::boost
)

shift_add_test(test.shift.math
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  SOURCEROOTS
    ${CMAKE_CURRENT_SOURCE_DIR}/test
  INCLUDEDIRS
    ${CMAKE_SOURCE_DIR}/shift/math/public
    ${CMAKE_SOURCE_DIR}/shift/serialization/public
  DEPENDENCIES
    shift.core
    shift.platform
    Boost::unit_test_framework
)
