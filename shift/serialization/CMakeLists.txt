shift_add_library(shift.serialization STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEPENDENCIES
    shift.parser.json
    shift.log
    shift.core
    shift.platform
)

shift_add_test(test.shift.serialization
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  SOURCEROOTS
    ${CMAKE_CURRENT_SOURCE_DIR}/test
  DEPENDENCIES
    shift.serialization
    shift.parser.json
    shift.log
    shift.core
    shift.platform
    Boost::unit_test_framework
    Boost::system
)
