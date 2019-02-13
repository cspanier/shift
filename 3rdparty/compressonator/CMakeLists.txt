find_package(OpenEXR)

shift_add_library(3rdparty.compressonator STATIC
  PUBLICSOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATESOURCEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/private
  # INCLUDEDIRS
  #   ${CMAKE_CURRENT_SOURCE_DIR}/compressonator
  # DEFINITIONS
  #   _SOFTWARE_ONLY_VERSION
  DEPENDENCIES
    OpenEXR::half
)
