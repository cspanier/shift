shift_add_library(3rdparty.amd-tootle STATIC
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  INCLUDEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private/RayTracer/JRT
    ${CMAKE_CURRENT_SOURCE_DIR}/private/RayTracer/Math
    ${CMAKE_CURRENT_SOURCE_DIR}/private/RayTracer
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  DEFINITIONS
    _SOFTWARE_ONLY_VERSION
)
