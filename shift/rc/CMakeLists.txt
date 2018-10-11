if (NOT NO_VULKAN)
  shift_add_library(shift.rc STATIC
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
      3rdparty.squish
      TIFF::TIFF
      JPEG::JPEG
      PNG::PNG
      BZip2::BZip2
      ZLIB::ZLIB
  )
endif()
