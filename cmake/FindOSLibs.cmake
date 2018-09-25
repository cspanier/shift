include(FindPackageHandleStandardArgs)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # add_custom_target(OSLibsNetwork)
  # add_library(OSLibs::network UNKNOWN IMPORTED)
  # set_property(TARGET OSLibs::network PROPERTY LINKER_LANGUAGE CXX)
  # add_dependencies(OSLibs::network OSLibsNetwork)

  if(OSLIBS_FOUND)
    set(HAVE_OSLIBS 1)
    set(OSLIBS_INCLUDE_DIRS)
    set(OSLIBS_LIBRARIES)
  endif()

  add_library(OSLibs::network INTERFACE IMPORTED)
  set_target_properties(OSLibs::network PROPERTIES
    IMPORTED_LIBNAME "")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  find_library(OSLIBS_IPHLPAPI_LIBRARY NAMES IPHLPAPI)
  mark_as_advanced(OSLIBS_IPHLPAPI_LIBRARY)

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSLibs REQUIRED_VARS
    OSLIBS_IPHLPAPI_LIBRARY)

  if(OSLIBS_FOUND)
    set(HAVE_OSLIBS 1)
    set(OSLIBS_INCLUDE_DIRS)
    set(OSLIBS_LIBRARIES ${OSLIBS_IPHLPAPI_LIBRARY})
  endif()

  if(IS_ABSOLUTE "${OSLIBS_IPHLPAPI_LIBRARY}")
    add_library(OSLibs::network UNKNOWN IMPORTED)
    set_target_properties(OSLibs::network PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_IPHLPAPI_LIBRARY}")
  else()
    add_library(OSLibs::network INTERFACE IMPORTED)
    set_target_properties(OSLibs::network PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_IPHLPAPI_LIBRARY}")
  endif()
endif()
