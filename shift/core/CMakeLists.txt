shift_add_library(shift.core STATIC
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  DEPENDENCIES
    shift.platform
    Boost::boost
)

# install(TARGETS shift.core DESTINATION cmake/shift/core EXPORT shift-core)
# install(EXPORT shift-core DESTINATION cmake/shift)

shift_add_test(test.shift.core
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  SOURCEROOTS
    ${CMAKE_CURRENT_SOURCE_DIR}/test
  DEPENDENCIES
    shift.core
    shift.platform
    Boost::unit_test_framework
    Boost::coroutine
    Boost::filesystem
    Boost::program_options
    Boost::iostreams
    Boost::regex
    Boost::serialization
    Boost::date_time
    Boost::locale
)
