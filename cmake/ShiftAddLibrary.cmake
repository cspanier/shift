macro(shift_add_library target)
  set(argument_names "ROOT;DOCDIRS;PUBLICSOURCEDIRS;PRIVATESOURCEDIRS;SOURCEEXTS")
  set(argument_names "${argument_names};SOURCES;PRECOMPILED_HEADER")
  set(argument_names "${argument_names};INCLUDEDIRS;DEFINITIONS;LIBRARIES")
  set(argument_names "${argument_names};DEPENDENCIES;CXXFLAGS;LFLAGS;VERSION")
  set(argument_names "${argument_names};EXCLUDE_FROM_BUILD")
  parse_arguments("${argument_names}" "STATIC;SHARED;OBJECT;INTERFACE" ${ARGN})
  message(STATUS "Adding library ${target}...")

  if(NOT ARG_STATIC AND NOT ARG_SHARED AND NOT ARG_OBJECT AND NOT ARG_INTERFACE)
    message(FATAL_ERROR "You need to specify the type of the library "
      "'${target}', which can be one of STATIC, SHARED, OBJECT, or INTERFACE.")
  endif()

  if("${ARG_ROOT}" STREQUAL "")
    set(ARG_ROOT ".")
  endif()

  if(NOT "${ARG_VERSION}" STREQUAL "")
    string(REGEX REPLACE "\\." "_" version_underscored "${ARG_VERSION}")
    set(version_tag ".${version_underscored}")
  else()
    set(version_tag "")
  endif()
  unset(sources)
  if("${ARG_SOURCEEXTS}" STREQUAL "")
    set(ARG_SOURCEEXTS
      # source files
      "*.h"
      "*.hpp"
      "*.hxx"
      "*.c"
      "*.cc"
      "*.cpp"
      "*.cxx"
      "*.inl"
      "*.def"
      # image files
      "*.png"
      "*.svg"
      "*.jpg"
      "*.bmp"
      "*.ico")
    if(ARG_SHARED)
      set(ARG_SOURCEEXTS ${ARG_SOURCEEXTS}
        # resource files
        "*.rc"
        "*.rc2")
    endif()
  endif()
  if(ARG_DOCDIRS)
    set(_docdirs ${ARG_DOCDIRS})
  else()
    set(_docdirs "${CMAKE_CURRENT_SOURCE_DIR}/doc")
  endif()
  find_sources(documentation_files
    ROOTS ${_docdirs}
    EXTS "*.md" "*.txt"
    GROUPPREFIX "doc/")
  if(ARG_PUBLICSOURCEDIRS)
    find_sources(public_sources
      ROOTS ${ARG_PUBLICSOURCEDIRS}
      EXTS ${ARG_SOURCEEXTS}
      GROUPPREFIX "public/"
      INSTALLDIR "${ARG_ROOT}/include/${PROJECT_NAME}${version_tag}")
    list(APPEND sources ${public_sources})
  endif()
  if(ARG_PRIVATESOURCEDIRS)
    find_sources(private_sources
      ROOTS ${ARG_PRIVATESOURCEDIRS}
      EXTS ${ARG_SOURCEEXTS}
      GROUPPREFIX "private/")
    list(APPEND sources ${private_sources})
  endif()
  list(APPEND sources ${ARG_SOURCES})
  if("${sources}" STREQUAL "")
    message(FATAL_ERROR "Target ${target} has no source files")
  endif()

  foreach(dependency ${ARG_DEPENDENCIES})
    if (TARGET ${dependency})
      list(APPEND ARG_LIBRARIES ${dependency})
    elseif(${dependency}_INCLUDE_DIRS OR
        ${dependency}_DEFINITIONS OR
        ${dependency}_LIBRARIES)
      if(${dependency}_INCLUDE_DIRS)
        list(APPEND ARG_INCLUDEDIRS ${${dependency}_INCLUDE_DIRS})
      endif()
      if(${dependency}_DEFINITIONS)
        list(APPEND ARG_DEFINITIONS ${${dependency}_DEFINITIONS})
      endif()
      if(${dependency}_LIBRARIES)
        list(APPEND ARG_LIBRARIES ${${dependency}_LIBRARIES})
      endif()
    else()
      message(FATAL_ERROR "Unknown dependency ${dependency} in target ${target}.")
    endif()
  endforeach()

  set(build_file_suffix "${version_tag}.${SHIFT_SYSTEM_PROCESSOR}.${SHIFT_COMPILER_ACRONYM}")

  set(definitions "${SHIFT_GLOBAL_DEFINITIONS}")
  list(APPEND definitions "${ARG_DEFINITIONS}")
  list(APPEND definitions $<$<CONFIG:Release>:_RELEASE>)
  list(APPEND definitions BUILD_FILE_SUFFIX="${build_file_suffix}")
  set(LFLAGS "${GLOBAL_LINK_FLAGS} ${ARG_LFLAGS}")

  if(ARG_STATIC)
    set(type STATIC)
  elseif(ARG_INTERFACE)
    set(type INTERFACE)
  elseif(ARG_OBJECT)
    set(type OBJECT)
  elseif(ARG_SHARED)
    set(type SHARED)
    # Automatically add a preprocessor macro to shared library builds
    STRING(TOUPPER "${target}_EXPORTS" export_define)
    STRING(REGEX REPLACE "\\." "_" export_define "${export_define}")
    list(APPEND definitions "${export_define}")
    unset(export_define)
  endif()

  if(NOT ARG_INTERFACE)
    add_library(${target} ${type} ${sources})
    target_link_libraries(${target} ${ARG_LIBRARIES})
    set_target_properties(${target} PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      PREFIX ""
      COMPILE_DEFINITIONS  # This property being ignored by MSVC CMake generator.
        "${definitions}"
      COMPILE_DEFINITIONS_DEBUG
        "${definitions}"
        "BUILD_CONFIG_DEBUG"
      COMPILE_DEFINITIONS_MINSIZEREL
        "${definitions}"
        "BUILD_CONFIG_MINSIZEREL"
      COMPILE_DEFINITIONS_RELEASE
        "${definitions}"
        "BUILD_CONFIG_RELEASE"
      COMPILE_DEFINITIONS_RELWITHDEBINFO
        "${definitions}"
        "BUILD_CONFIG_RELWITHDEBINFO"
      COMPILE_FLAGS "${ARG_CXXFLAGS}"
      LINK_FLAGS "${LFLAGS}"
      LINK_FLAGS_DEBUG "${GLOBAL_LINK_FLAGS_DEBUG}"
      LINK_FLAGS_RELEASE "${GLOBAL_LINK_FLAGS_RELEASE}"
      # LINKER_LANGUAGE CXX

      #LIBRARY_OUTPUT_DIRECTORY
      #  "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
      #LIBRARY_OUTPUT_DIRECTORY_DEBUG
      #  "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
      #LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL
      #  "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
      #LIBRARY_OUTPUT_DIRECTORY_RELEASE
      #  "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
      #LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO
      #  "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"

      #ARCHIVE_OUTPUT_DIRECTORY
      #  "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
      #ARCHIVE_OUTPUT_DIRECTORY_DEBUG
      #  "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
      #ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL
      #  "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
      #ARCHIVE_OUTPUT_DIRECTORY_RELEASE
      #  "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
      #ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO
      #  "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"

      RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin"
      RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin.debug"
      RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
        "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin.minsizerel"
      RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin"
      RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
        "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin.relwithdeb"

      OUTPUT_NAME
        "${target}${build_file_suffix}"
      OUTPUT_NAME_DEBUG
        "${target}${build_file_suffix}"
      OUTPUT_NAME_MINSIZEREL
        "${target}${build_file_suffix}"
      OUTPUT_NAME_RELEASE
        "${target}${build_file_suffix}"
      OUTPUT_NAME_RELWITHDEBINFO
        "${target}${build_file_suffix}"

      # PDB_NAME<_CONFIG> seems to have no effect.
      PDB_NAME
        "${target}${build_file_suffix}"
      PDB_NAME_DEBUG
        "${target}${build_file_suffix}"
      PDB_NAME_MINSIZEREL
        "${target}${build_file_suffix}"
      PDB_NAME_RELEASE
        "${target}${build_file_suffix}"
      PDB_NAME_RELWITHDEBINFO
        "${target}${build_file_suffix}"

      VS_WINDOWS_TARGET_PLATFORM_VERSION
        "${CMAKE_SYSTEM_VERSION}"
    )
    if(ARG_STATIC OR ARG_OBJECT)
      set_target_properties(${target} PROPERTIES
        # PDB_OUTPUT_DIRECTORY<_CONFIG> seems to have no effect.
        PDB_OUTPUT_DIRECTORY
          "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
        PDB_OUTPUT_DIRECTORY_DEBUG
          "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib.debug"
        PDB_OUTPUT_DIRECTORY_MINSIZEREL
          "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib.minsizerel"
        PDB_OUTPUT_DIRECTORY_RELEASE
          "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
        PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO
          "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib.relwithdeb"
      )
    elseif(ARG_SHARED)
      set_target_properties(${target} PROPERTIES
        # PDB_OUTPUT_DIRECTORY<_CONFIG> seems to have no effect.
        PDB_OUTPUT_DIRECTORY
          "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin"
        PDB_OUTPUT_DIRECTORY_DEBUG
          "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin.debug"
        PDB_OUTPUT_DIRECTORY_MINSIZEREL
          "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin.minsizerel"
        PDB_OUTPUT_DIRECTORY_RELEASE
          "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin"
        PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO
          "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/bin.relwithdeb"
      )
    endif()
    set_target_folder(${target})
  else()
    add_library(${target} INTERFACE)
  endif()

  foreach(dependency ${ARG_DEPENDENCIES})
    if (TARGET ${dependency})
      add_dependencies(${target} ${dependency})
    endif()
  endforeach()

  if(ARG_EXCLUDE_FROM_BUILD)
    exclude_target_from_build(${target})
  else()
    if(NOT ARG_OBJECT)
      install(TARGETS ${target}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/lib"
        EXPORT "${CMAKE_PROJECT_NAME}")
      install(EXPORT "${CMAKE_PROJECT_NAME}"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${ARG_ROOT}/cmake")
    endif()
  endif()

  set(private_include_dirs)
  set(public_include_dirs)
  list(APPEND private_include_dirs ${ARG_PRIVATESOURCEDIRS})
  list(APPEND private_include_dirs ${ARG_INCLUDEDIRS})
  list(APPEND private_include_dirs "${CMAKE_PREFIX_PATH}/include")
  list(APPEND public_include_dirs ${ARG_PUBLICSOURCEDIRS})
  if(NOT ARG_INTERFACE)
    target_include_directories(${target}
      PRIVATE ${private_include_dirs}
      PUBLIC $<BUILD_INTERFACE:${public_include_dirs}>)
  else()
    target_include_directories(${target}
      INTERFACE $<BUILD_INTERFACE:${public_include_dirs}>)
  endif()

  if(ARG_PRECOMPILED_HEADER)
    shift_add_precompiled_header(${target} "${sources}" "${ARG_PRECOMPILED_HEADER}")
    unset(ARG_PRECOMPILED_HEADER)
  endif()

  unset(version_tag)
  unset(sources)
  unset(ARG_ROOT)
  unset(ARG_PUBLICSOURCEDIRS)
  unset(ARG_PRIVATESOURCEDIRS)
  unset(ARG_SOURCEEXTS)
  unset(ARG_SOURCES)
  unset(ARG_PRECOMPILED_HEADER)
  unset(ARG_INCLUDEDIRS)
  unset(ARG_DEFINITIONS)
  unset(ARG_LIBRARIES)
  unset(ARG_DEPENDENCIES)
  unset(ARG_CXXFLAGS)
  unset(ARG_LFLAGS)
  unset(ARG_VERSION)
  unset(ARG_EXCLUDE_FROM_BUILD)
  unset(ARG_STATIC)
  unset(ARG_SHARED)
  unset(ARG_OBJECT)
  unset(ARG_INTERFACE)
endmacro()
