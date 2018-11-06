if (NOT NO_VULKAN)
  shift_add_library(shift.render.vk STATIC
    VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
    PUBLICSOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/public
    PRIVATESOURCEDIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/private
    PRECOMPILED_HEADER
      "${CMAKE_CURRENT_SOURCE_DIR}/private/shift/render/vk/precompiled.hpp"
    DEPENDENCIES
      shift.scene
      shift.resource
      shift.livedebug
      shift.parser.json
      shift.serialization2
      shift.task
      shift.log
      shift.crypto
      shift.math
      shift.core
      shift.platform
      Boost::boost
      Boost::filesystem
      Boost::iostreams
      GLFW3
      Vulkan::Vulkan
      BZip2::BZip2
      ZLIB::ZLIB
  )

  shift_add_test(test.shift.render.vk
    VERSION ${SHIFT_VERSION_MAJOR}.${SHIFT_VERSION_MINOR}
    SOURCEROOTS
      ${CMAKE_CURRENT_SOURCE_DIR}/test
    DEPENDENCIES
      shift.render.vk
      shift.resource
      Boost::unit_test_framework
  )
endif()
