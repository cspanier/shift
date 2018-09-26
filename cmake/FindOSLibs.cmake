include(FindPackageHandleStandardArgs)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(OSLIBS_FOUND)
    set(HAVE_OSLIBS 1)
    set(OSLIBS_INCLUDE_DIRS)
    set(OSLIBS_LIBRARIES)
  endif()

  add_library(OSLibs::iphlpapi INTERFACE IMPORTED)
  set_target_properties(OSLibs::iphlpapi PROPERTIES
    IMPORTED_LIBNAME "")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  find_library(OSLIBS_IPHLPAPI_LIBRARY NAMES iphlpapi.lib)
  mark_as_advanced(OSLIBS_IPHLPAPI_LIBRARY)

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSLibs REQUIRED_VARS
    OSLIBS_IPHLPAPI_LIBRARY)

  if(OSLIBS_FOUND)
    set(HAVE_OSLIBS 1)
    set(OSLIBS_INCLUDE_DIRS)
    set(OSLIBS_LIBRARIES ${OSLIBS_IPHLPAPI_LIBRARY})
  endif()

  if(IS_ABSOLUTE "${OSLIBS_IPHLPAPI_LIBRARY}")
    add_library(OSLibs::iphlpapi UNKNOWN IMPORTED)
    set_target_properties(OSLibs::iphlpapi PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_IPHLPAPI_LIBRARY}")
  else()
    add_library(OSLibs::iphlpapi INTERFACE IMPORTED)
    set_target_properties(OSLibs::iphlpapi PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_IPHLPAPI_LIBRARY}")
  endif()
endif()
