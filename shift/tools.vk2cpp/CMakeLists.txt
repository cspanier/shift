shift_add_executable(shift.tools.vk2cpp
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  SOURCEDIRS
    "${CMAKE_CURRENT_SOURCE_DIR}"
  DEPENDENCIES
    shift.application
    shift.serialization
    shift.parser.json
    shift.parser.xml
    shift.log
    shift.crypto
    shift.core
    shift.platform
    Boost::boost
    Boost::filesystem
    Boost::program_options
)
