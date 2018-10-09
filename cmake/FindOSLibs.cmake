include(FindPackageHandleStandardArgs)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # add_custom_target(OSLibsNetwork)
  # add_library(OSLibs::iphlpapi UNKNOWN IMPORTED)
  # set_property(TARGET OSLibs::iphlpapi PROPERTY LINKER_LANGUAGE CXX)
  # add_dependencies(OSLibs::iphlpapi OSLibsNetwork)

  # if(OSLIBS_FOUND)
    set(HAVE_OSLIBS 1)
    set(OSLIBS_INCLUDE_DIRS)
    set(OSLIBS_LIBRARIES)
  # endif()

  add_library(OSLibs::iphlpapi INTERFACE IMPORTED)
  set_target_properties(OSLibs::iphlpapi PROPERTIES
    IMPORTED_LIBNAME "")

  add_library(OSLibs::winsock2 INTERFACE IMPORTED)
  set_target_properties(OSLibs::winsock2 PROPERTIES
    IMPORTED_LIBNAME "")
    
  add_library(OSLibs::mswsock INTERFACE IMPORTED)
  set_target_properties(OSLibs::mswsock PROPERTIES
    IMPORTED_LIBNAME "")

  add_library(OSLibs::version INTERFACE IMPORTED)
  set_target_properties(OSLibs::version PROPERTIES
    IMPORTED_LIBNAME "")

  add_library(OSLibs::dbghelp INTERFACE IMPORTED)
  set_target_properties(OSLibs::dbghelp PROPERTIES
    IMPORTED_LIBNAME "")

  add_library(OSLibs::wininet INTERFACE IMPORTED)
  set_target_properties(OSLibs::wininet PROPERTIES
    IMPORTED_LIBNAME "")

  add_library(OSLibs::comctl32 INTERFACE IMPORTED)
  set_target_properties(OSLibs::comctl32 PROPERTIES
    IMPORTED_LIBNAME "")

  add_library(OSLibs::shlwapi INTERFACE IMPORTED)
  set_target_properties(OSLibs::shlwapi PROPERTIES
    IMPORTED_LIBNAME "")

  add_library(OSLibs::secur32 INTERFACE IMPORTED)
  set_target_properties(OSLibs::secur32 PROPERTIES
    IMPORTED_LIBNAME "")

  add_library(OSLibs::winmm INTERFACE IMPORTED)
  set_target_properties(OSLibs::winmm PROPERTIES
    IMPORTED_LIBNAME "")

  add_library(OSLibs::dwmapi INTERFACE IMPORTED)
  set_target_properties(OSLibs::dwmapi PROPERTIES
    IMPORTED_LIBNAME "")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  find_library(OSLIBS_IPHLPAPI_LIBRARY NAMES IPHLPAPI)
  mark_as_advanced(OSLIBS_IPHLPAPI_LIBRARY)

  find_library(OSLIBS_WS32_32_LIBRARY NAMES Ws2_32.lib)
  mark_as_advanced(OSLIBS_WS32_32_LIBRARY)

  find_library(OSLIBS_MSWSOCK_LIBRARY NAMES Mswsock.lib)
  mark_as_advanced(OSLIBS_MSWSOCK_LIBRARY)

  find_library(OSLIBS_VERSION_LIBRARY NAMES version.lib)
  mark_as_advanced(OSLIBS_VERSION_LIBRARY)

  find_library(OSLIBS_DBGHELP_LIBRARY NAMES DbgHelp.lib)
  mark_as_advanced(OSLIBS_DBGHELP_LIBRARY)

  find_library(OSLIBS_WININET_LIBRARY NAMES Wininet.lib)
  mark_as_advanced(OSLIBS_WININET_LIBRARY)
  
  find_library(OSLIBS_COMCTL32_LIBRARY NAMES Comctl32.lib)
  mark_as_advanced(OSLIBS_COMCTL32_LIBRARY)
  
  find_library(OSLIBS_SHLWAPI_LIBRARY NAMES Shlwapi.lib)
  mark_as_advanced(OSLIBS_SHLWAPI_LIBRARY)
  
  find_library(OSLIBS_SECUR32_LIBRARY NAMES Secur32.lib)
  mark_as_advanced(OSLIBS_SECUR32_LIBRARY)
  
  find_library(OSLIBS_WINMM_LIBRARY NAMES Dwmapi.lib)
  mark_as_advanced(OSLIBS_WINMM_LIBRARY)
  
  find_library(OSLIBS_DWMAPI_LIBRARY NAMES Dwmapi.lib)
  mark_as_advanced(OSLIBS_DWMAPI_LIBRARY)

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSLibs REQUIRED_VARS
    OSLIBS_IPHLPAPI_LIBRARY
    OSLIBS_WS32_32_LIBRARY
    OSLIBS_MSWSOCK_LIBRARY
    OSLIBS_VERSION_LIBRARY
    OSLIBS_DBGHELP_LIBRARY
    OSLIBS_WININET_LIBRARY
    OSLIBS_COMCTL32_LIBRARY
    OSLIBS_SHLWAPI_LIBRARY
    OSLIBS_SECUR32_LIBRARY
    OSLIBS_WINMM_LIBRARY
    OSLIBS_DWMAPI_LIBRARY
  )

  if(OSLIBS_FOUND)
    set(HAVE_OSLIBS 1)
    set(OSLIBS_INCLUDE_DIRS)
    set(OSLIBS_LIBRARIES
      ${OSLIBS_IPHLPAPI_LIBRARY}
      ${OSLIBS_WS32_32_LIBRARY}
      ${OSLIBS_MSWSOCK_LIBRARY}
      ${OSLIBS_VERSION_LIBRARY}
      ${OSLIBS_DBGHELP_LIBRARY}
      ${OSLIBS_WININET_LIBRARY}
      ${OSLIBS_COMCTL32_LIBRARY}
      ${OSLIBS_SHLWAPI_LIBRARY}
      ${OSLIBS_SECUR32_LIBRARY}
      ${OSLIBS_WINMM_LIBRARY}
      ${OSLIBS_DWMAPI_LIBRARY}
    )
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

  if(IS_ABSOLUTE "${OSLIBS_WS32_32_LIBRARY}")
    add_library(OSLibs::winsock2 UNKNOWN IMPORTED)
    set_target_properties(OSLibs::winsock2 PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_WS32_32_LIBRARY}")
  else()
    add_library(OSLibs::winsock2 INTERFACE IMPORTED)
    set_target_properties(OSLibs::winsock2 PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_WS32_32_LIBRARY}")
  endif()
  
  if(IS_ABSOLUTE "${OSLIBS_MSWSOCK_LIBRARY}")
    add_library(OSLibs::mswsock UNKNOWN IMPORTED)
    set_target_properties(OSLibs::mswsock PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_MSWSOCK_LIBRARY}")
  else()
    add_library(OSLibs::mswsock INTERFACE IMPORTED)
    set_target_properties(OSLibs::mswsock PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_MSWSOCK_LIBRARY}")
  endif()

  if(IS_ABSOLUTE "${OSLIBS_VERSION_LIBRARY}")
    add_library(OSLibs::version UNKNOWN IMPORTED)
    set_target_properties(OSLibs::version PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_VERSION_LIBRARY}")
  else()
    add_library(OSLibs::version INTERFACE IMPORTED)
    set_target_properties(OSLibs::version PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_VERSION_LIBRARY}")
  endif()

  if(IS_ABSOLUTE "${OSLIBS_DBGHELP_LIBRARY}")
    add_library(OSLibs::dbghelp UNKNOWN IMPORTED)
    set_target_properties(OSLibs::dbghelp PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_DBGHELP_LIBRARY}")
  else()
    add_library(OSLibs::dbghelp INTERFACE IMPORTED)
    set_target_properties(OSLibs::dbghelp PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_DBGHELP_LIBRARY}")
  endif()

  if(IS_ABSOLUTE "${OSLIBS_WININET_LIBRARY}")
    add_library(OSLibs::wininet UNKNOWN IMPORTED)
    set_target_properties(OSLibs::wininet PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_WININET_LIBRARY}")
  else()
    add_library(OSLibs::wininet INTERFACE IMPORTED)
    set_target_properties(OSLibs::wininet PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_WININET_LIBRARY}")
  endif()

  if(IS_ABSOLUTE "${OSLIBS_COMCTL32_LIBRARY}")
    add_library(OSLibs::comctl32 UNKNOWN IMPORTED)
    set_target_properties(OSLibs::comctl32 PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_COMCTL32_LIBRARY}")
  else()
    add_library(OSLibs::comctl32 INTERFACE IMPORTED)
    set_target_properties(OSLibs::comctl32 PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_COMCTL32_LIBRARY}")
  endif()

  if(IS_ABSOLUTE "${OSLIBS_SHLWAPI_LIBRARY}")
    add_library(OSLibs::shlwapi UNKNOWN IMPORTED)
    set_target_properties(OSLibs::shlwapi PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_SHLWAPI_LIBRARY}")
  else()
    add_library(OSLibs::shlwapi INTERFACE IMPORTED)
    set_target_properties(OSLibs::shlwapi PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_SHLWAPI_LIBRARY}")
  endif()

  if(IS_ABSOLUTE "${OSLIBS_SECUR32_LIBRARY}")
    add_library(OSLibs::secur32 UNKNOWN IMPORTED)
    set_target_properties(OSLibs::secur32 PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_SECUR32_LIBRARY}")
  else()
    add_library(OSLibs::secur32 INTERFACE IMPORTED)
    set_target_properties(OSLibs::secur32 PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_SECUR32_LIBRARY}")
  endif()

  if(IS_ABSOLUTE "${OSLIBS_WINMM_LIBRARY}")
    add_library(OSLibs::winmm UNKNOWN IMPORTED)
    set_target_properties(OSLibs::winmm PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_WINMM_LIBRARY}")
  else()
    add_library(OSLibs::winmm INTERFACE IMPORTED)
    set_target_properties(OSLibs::winmm PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_WINMM_LIBRARY}")
  endif()

  if(IS_ABSOLUTE "${OSLIBS_DWMAPI_LIBRARY}")
    add_library(OSLibs::dwmapi UNKNOWN IMPORTED)
    set_target_properties(OSLibs::dwmapi PROPERTIES
      IMPORTED_LOCATION "${OSLIBS_DWMAPI_LIBRARY}")
  else()
    add_library(OSLibs::dwmapi INTERFACE IMPORTED)
    set_target_properties(OSLibs::dwmapi PROPERTIES
      IMPORTED_LIBNAME "${OSLIBS_DWMAPI_LIBRARY}")
  endif()
endif()
