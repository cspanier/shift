shift_add_executable(shift.tools.protogen
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  SOURCEDIRS
    "${CMAKE_CURRENT_SOURCE_DIR}"
  DEPENDENCIES
    shift.application
    shift.serialization2
    shift.parser.proto
    shift.parser.json
    shift.crypto
    shift.log
    shift.core
    shift.platform
    Boost::program_options
)
