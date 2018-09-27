shift_add_library(shift.crypto STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEPENDENCIES
    shift.core
    shift.platform
)

shift_add_test(test.shift.crypto
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  SOURCEROOTS
    ${CMAKE_CURRENT_SOURCE_DIR}/test
  DEPENDENCIES
    shift.crypto
    shift.core
    shift.platform
    Boost::unit_test_framework
)
