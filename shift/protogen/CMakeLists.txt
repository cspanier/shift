shift_add_executable(shift.protogen
  VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
  SOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEPENDENCIES
    shift.application
    shift.proto
    shift.serialization
    shift.parser.json
    shift.crypto
    shift.log
    shift.core
    shift.platform
    Boost::program_options
)
