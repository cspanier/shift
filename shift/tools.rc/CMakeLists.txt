if(NOT NO_RC)
  shift_add_executable(shift.tools.rc
    VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
    SOURCEDIRS
      "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDENCIES
      shift.application
      shift.rc
      # shift.graphics
      shift.resource_db
      shift.parser.json
      shift.serialization2
      shift.log
      shift.crypto
      shift.task
      shift.math
      shift.core
      shift.platform
      Boost::boost
      Boost::iostreams
      Boost::context
  )
endif()
