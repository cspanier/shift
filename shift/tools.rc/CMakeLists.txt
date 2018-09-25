if (NOT NO_VULKAN)
  shift_add_library(3rdparty.amd-tootle STATIC
    PUBLICSOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/amd-tootle
    PRIVATESOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/amd-tootle
    INCLUDEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/amd-tootle/RayTracer/JRT
      ${CMAKE_CURRENT_SOURCE_DIR}/amd-tootle/RayTracer/Math
      ${CMAKE_CURRENT_SOURCE_DIR}/amd-tootle/RayTracer
      ${CMAKE_CURRENT_SOURCE_DIR}/amd-tootle/
    DEFINITIONS
      _SOFTWARE_ONLY_VERSION
  )

  shift_add_library(shift.tools.rc STATIC
    VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
    PUBLICSOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/public
    PRIVATESOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/private
    INCLUDEDIRS
      ${CMAKE_SOURCE_DIR}/shift/math/public
    DEPENDENCIES
      # shift.graphics
      shift.resource
      shift.parser.json
      shift.serialization2
      shift.log
      shift.crypto
      shift.task
      shift.core
      shift.platform
      Boost::boost
      # SPIRV_CROSS
      ShaderC::ShaderC
      3rdparty.amd-tootle
      TIFF::TIFF
      JPEG::JPEG
      PNG::PNG
      BZip2::BZip2
      ZLIB::ZLIB
  )
endif()
