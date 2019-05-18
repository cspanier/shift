if (NOT NO_VULKAN)
  shift_add_library(shift.rc STATIC
    VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
    PUBLICSOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/public
    PRIVATESOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/private
    DEPENDENCIES
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
      ShaderC::core
      ShaderC::util
      Glslang::hlsl
      Glslang::ogl
      Glslang::core
      Glslang::osdependent
      SpirV::core
      SpirVTools::optimizer
      SpirVTools::core
      SpirVCross::util
      SpirVCross::core
      3rdparty.amd-tootle
      3rdparty.squish-ccr
      JPEG::JPEG
      PNG::PNG
      TIFF::TIFF
      LCMS::LCMS2
      BZip2::BZip2
      ZLIB::ZLIB
  )

  shift_add_test(test.shift.rc
    VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
    SOURCEROOTS
      ${CMAKE_CURRENT_SOURCE_DIR}/test
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
      Boost::unit_test_framework
  )
endif()
