# clears all passed variables
macro(clear_vars)
  foreach(var ${ARGN})
    unset(${var} CACHE)
  endforeach()
endmacro()

###############################################################################

macro(parse_arguments arg_names option_names)
  set(DEFAULT_ARGS)
  foreach(arg_name ${arg_names})
    set(ARG_${arg_name})
  endforeach()
  foreach(option ${option_names})
    set(ARG_${option} FALSE)
  endforeach()

  set(current_arg_name DEFAULT_ARGS)
  set(current_arg_list)
  foreach(arg ${ARGN})
    set(larg_names ${arg_names})
    list(FIND larg_names "${arg}" is_arg_name)
    if(is_arg_name GREATER -1)
      set(ARG_${current_arg_name} ${current_arg_list})
      set(current_arg_name ${arg})
      set(current_arg_list)
    else()
      set(loption_names ${option_names})
      list(FIND loption_names "${arg}" is_option)
      if(is_option GREATER -1)
        set(ARG_${arg} TRUE)
      else()
        set(current_arg_list ${current_arg_list} ${arg})
      endif()
    endif()
  endforeach()
  set(ARG_${current_arg_name} ${current_arg_list})
endmacro()

###############################################################################

# This macro globs c++ sources and groups them for msvs to reflect the
# directory structure.
macro(find_sources result)
  parse_arguments("ROOTS;EXTS;GROUPPREFIX;INSTALLDIR" "" ${ARGN})
  if("${ARG_EXTS}" STREQUAL "")
    message(FATAL_ERROR "Missing argument EXTS")
  endif()

  set(${result})
  foreach (root ${ARG_ROOTS})
    foreach(source_ext ${ARG_EXTS})
      # glob files from directory.
      file(GLOB_RECURSE sources_of_dir RELATIVE "${root}/"
        "${root}/${source_ext}")

      foreach(source ${sources_of_dir})
        # add to list of sources.
        list(APPEND ${result} "${root}/${source}")

        # add source group, that reflects the directory structure.
        get_filename_component(source_path "${source}" PATH)
        string(REGEX REPLACE "/" "\\\\" vs_source_path "${ARG_GROUPPREFIX}${source_path}")
        source_group("${vs_source_path}" FILES "${root}/${source}")

        if(ARG_INSTALLDIR)
          install(FILES "${root}/${source}"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/${ARG_INSTALLDIR}/${source_path}")
        endif()
      endforeach()
    endforeach()
  endforeach()
  unset(ARG_ROOTS)
  unset(ARG_EXTS)
  unset(ARG_DIRS)
  unset(ARG_INSTALLDIR)
endmacro()

###############################################################################

# This macro excludes c++ sources from the build. It is useful for msvs project
# files to display other than windows platform files.
macro(exclude_sources)
  # loop through all optional arguments.
  foreach(source_file_pattern ${ARGN})
    # message(STATUS "Excluding ${source_file_pattern}")
    file(GLOB source_file "${source_file_pattern}")

    # loop through all files to exclude.
    foreach(source_file ${source_file})
      set_source_files_properties(${source_file} PROPERTIES
        HEADER_FILE_ONLY TRUE)
    endforeach()
  endforeach()
endmacro()

###############################################################################

# This macro excludes c++ sources from the build. It is useful for msvs project
# files to display other than windows platform files.
macro(exclude_source_dirs)
  # loop through all optional arguments.
  foreach(source_dir ${ARGN})
    # glob files from directory.
    message(STATUS "Excluding *.cpp from ${source_dir}")
    file(GLOB sources_of_dir "${source_dir}/*.cpp")

    # loop through all files to exclude.
    foreach(source_file ${sources_of_dir})
      set_source_files_properties(${source_file} PROPERTIES
        HEADER_FILE_ONLY TRUE)
    endforeach()
  endforeach()
endmacro()

###############################################################################

macro(exclude_target_from_build target_name)
  set_target_properties(
    ${target_name} PROPERTIES
    EXCLUDE_FROM_DEFAULT_BUILD TRUE
    EXCLUDE_FROM_ALL TRUE
  )
endmacro()

###############################################################################

macro(set_default_search_paths)
  # ToDo: This is legacy stuff which should not be used any more.
  if(WIN32)
    set(SHIFT_DEFAULT_SYSTEM_HEADER_PATHS
      $ENV{INCLUDE}
    )
    set(SHIFT_DEFAULT_SYSTEM_LIBRARY_PATHS $ENV{LIB})
    set(SHIFT_DEFAULT_SYSTEM_LIBRARY_PATH_SUFFIXES)
  elseif(UNIX)
    set(SHIFT_DEFAULT_SYSTEM_HEADER_PATHS
      "~/"
      "/usr/local/include"
      "/usr/include"
    )

    set(SHIFT_DEFAULT_SYSTEM_LIBRARY_PATHS
      "~/"
      "/usr/local"
      "/usr"
    )
    if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
      set(SHIFT_DEFAULT_SYSTEM_LIBRARY_PATH_SUFFIXES
        "lib/x86_64-linux-gnu"
        "lib64"
        "lib"
      )
    elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_32")
      set(SHIFT_DEFAULT_SYSTEM_LIBRARY_PATH_SUFFIXES
        "lib/x86_32-linux-gnu"
        "lib32"
        "lib"
      )
    else()
      set(SHIFT_DEFAULT_SYSTEM_LIBRARY_PATH_SUFFIXES "lib")
    endif()
  endif()
endmacro()

###############################################################################

# Automatically sets the FOLDER target property according to the target name
# using dots ('.') as separator characters.
macro(set_target_folder target)
  string(TOLOWER "${target}" target_folder)
  # Replace name separators with slashes (a.b.c.3d.1.0 -> a/b/c.3d.1.0)
  string(REGEX REPLACE "\\.([a-zA-Z][^\\.]*)" "/\\1" target_folder "${target_folder}")
  # Handle special names which start with a number (a/b/c.3d.1.0 -> a/b/c/3d.1.0)
  string(REGEX REPLACE "\\.([0-9]+[^\\.0-9]+)" "/\\1" target_folder "${target_folder}")
  # Split off last name ("a/b/c/3d.1.0" -> "a/b/c")
  string(REGEX REPLACE "(.*)(/[^/]+)" "\\1" target_folder "${target_folder}")
  if(NOT "${target_folder}" STREQUAL "")
    set_target_properties(${target} PROPERTIES
      FOLDER "${target_folder}")
  endif()
endmacro()

###############################################################################

macro(shift_find_package name)
  include("${CMAKE_SOURCE_DIR}/cmake/Lib${name}.cmake")
endmacro()

###############################################################################

# Includes all subdirectories which directly contain a CMakeLists.txt file.
macro(shift_add_subdirectories)
  file(GLOB folders RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*")
  foreach(folder ${folders})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${folder}" AND
      EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${folder}/CMakeLists.txt")
      add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${folder})
    endif()
  endforeach()
endmacro()

###############################################################################

macro(shift_add_precompiled_header target sources precompiled_header)
  # get_target_property(cxxflags ${target} COMPILE_FLAGS)
  STRING(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1.cpp" precompiled_source "${precompiled_header}")
  STRING(REGEX REPLACE "^.+/([^/]+)$" "\\1" precompiled_header_name "${precompiled_header}")
  if(MSVC AND SHIFT_CLANG_TIDY_ON_MSVC)
    # Don't use PCHs when using clang-tidy.
    foreach(sourcefile ${sources})
      set_source_files_properties("${sourcefile}" PROPERTIES
        COMPILE_FLAGS "/FI\"${precompiled_header}\"")
    endforeach()
  elseif(MSVC AND NOT CLANG)
    # message(STATUS "PCH: ${precompiled_header} ${precompiled_source} ${precompiled_header_name}")
    if(NOT EXISTS "${precompiled_source}")
      message(WARNING "MSVC requires a source file to support precompiled headers, which does not exist.")
    endif()
    # Precompiled Header compiler flags:
    #   /Y(c|u)"StdAfx.h"(C)reate/(U)se Precompiled Header
    #   /Fp"StdAfx.h.pch" PCH file
    #   /FI"StdAfx.h"     Forced include file
    foreach(sourcefile ${sources})
      if("${sourcefile}" STREQUAL "${precompiled_source}")
        set_source_files_properties("${sourcefile}" PROPERTIES
          COMPILE_FLAGS "/Zm256 /Yc\"${precompiled_header_name}\" /Fp\"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/precompiled.pch\"")
      elseif("${sourcefile}" MATCHES ".+\\.cpp$")
        set_source_files_properties("${sourcefile}" PROPERTIES
          COMPILE_FLAGS "/FI\"${precompiled_header}\" /Zm256 /Yu\"${precompiled_header}\" /Fp\"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/precompiled.pch\"")
      endif()
    endforeach()
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  #  foreach(sourcefile ${sources})
  #    if("${sourcefile}" MATCHES ".+\\.cpp$")
  #      set_source_files_properties("${sourcefile}" PROPERTIES
  #        COMPILE_FLAGS "-include \"${precompiled_header}\"")
  #    endif()
  #  endforeach()
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  #  set(pch_tmp_dir "${CMAKE_CURRENT_BINARY_DIR}/${target}.pch")
  #  set(pch_copy "${pch_tmp_dir}/${pch_header_filename}")
  #  if(CMAKE_COMPILER_IS_GNUCXX)
  #    set(pch_binary "${pch_tmp_dir}/${pch_header_filename}.pch")
  #  else()
  #    set(pch_binary "${pch_tmp_dir}/${pch_header_filename}.gch")
  #  endif()
  #
  #  # Add C++ generic and build type dependent compiler flags.
  #  if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  #    set(cxxflags ${cxxflags} ${CMAKE_CXX_FLAGS})
  #  else()
  #    string(TOUPPER CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}
  #      CMAKE_CXX_FLAGS_BUILD_TYPE)
  #    set(cxxflags ${cxxflags} ${CMAKE_CXX_FLAGS} ${${CMAKE_CXX_FLAGS_BUILD_TYPE}})
  #  endif()
  #  string(REPLACE " " ";" cxxflags "${cxxflags}")
  #
  #  # Add include directories.
  #  list(APPEND cxxflags "-I${pch_tmp_dir}")
  #  get_property(_include_directories_list
  #    DIRECTORY ${cmake_current_source_dir}
  #    PROPERTY INCLUDE_DIRECTORIES)
  #  foreach(item ${_include_directories_list})
  #    list(APPEND cxxflags "-I${item}")
  #  endforeach()
  #
  #  # Add generic compile definitions.
  #  get_target_property(DEFINITIONS ${target} COMPILE_DEFINITIONS)
  #  foreach(item ${DEFINITIONS})
  #    list(APPEND cxxflags "-D${item}")
  #  endforeach()
  #
  #  # Add build type dependent compile definitions.
  #  if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  #    message(WARNING "No build type set.")
  #  else()
  #    string(TOUPPER COMPILE_DEFINITIONS_${CMAKE_BUILD_TYPE}
  #      COMPILE_DEFINITIONS_BUILD_TYPE)
  #    get_target_property(DEFINITIONS_BUILD_TYPE ${target} ${COMPILE_DEFINITIONS_BUILD_TYPE})
  #    foreach(item ${DEFINITIONS_BUILD_TYPE})
  #      list(APPEND cxxflags "-D${item}")
  #    endforeach()
  #  endif()
  #
  #  add_custom_command(
  #    OUTPUT ${pch_binary}
  #    COMMAND ${CMAKE_COMMAND} -E make_directory ${pch_tmp_dir}
  #    COMMAND ${CMAKE_COMMAND} -E copy ${pch_header} ${pch_copy}
  #    COMMAND ${CMAKE_CXX_COMPILER}
  #        ${cxxflags}
  #        -x c++-header
  #        -o ${pch_binary}
  #        ${pch_header}
  #    DEPENDS ${pch_header}
  #    VERBATIM
  #  )
  #  add_custom_target(${target}.pch
  #    DEPENDS ${pch_binary})
  #  add_dependencies(${target} ${target}.pch)
  #  set_target_properties(${target} PROPERTIES
  #    COMPILE_FLAGS "-include ${pch_copy} -Winvalid-pch")
  else()
    message(STATUS "The selected compiler does not support PCHs for target ${target}.")
  endif()
endmacro()

###############################################################################

macro(hexchar2dec result char)
  if(${char} MATCHES "[0-9]")
    set(${result} ${char})
  elseif(${char} MATCHES "[aA]")
    set(${result} 10)
  elseif(${char} MATCHES "[bB]")
    set(${result} 11)
  elseif(${char} MATCHES "[cC]")
    set(${result} 12)
  elseif(${char} MATCHES "[dD]")
    set(${result} 13)
  elseif(${char} MATCHES "[eE]")
    set(${result} 14)
  elseif(${char} MATCHES "[Ff]")
    set(${result} 15)
  else()
    MESSAGE(FATAL_ERROR "Invalid format for hexidecimal character: '${char}'")
  endif()
endmacro()

macro(dec2hexchar result value)
  if (${value} LESS 10)
    set(${result} ${value})
  elseif(${value} EQUAL 10)
    set(${result} "A")
  elseif(${value} EQUAL 11)
    set(${result} "B")
  elseif(${value} EQUAL 12)
    set(${result} "C")
  elseif(${value} EQUAL 13)
    set(${result} "D")
  elseif(${value} EQUAL 14)
    set(${result} "E")
  elseif(${value} EQUAL 15)
    set(${result} "F")
  else()
    MESSAGE(FATAL_ERROR "Invalid format for hexidecimal value: ${value}")
  endif()
endmacro()

macro(combine_hashes result hash1 hash2)
  string(LENGTH "${hash1}" hash1length)
  string(LENGTH "${hash2}" hash2length)
  if(NOT ${hash1length} EQUAL ${hash2length})
    MESSAGE(FATAL_ERROR "Cannot combine hashes of different length")
  endif()

  set(index 0)
  set(${result} "")
  while (index LESS hash1length)
    string(SUBSTRING "${hash1}" ${index} 1 nibble1)
    string(SUBSTRING "${hash2}" ${index} 1 nibble2)

    hexchar2dec(nibble1 "${nibble1}")
    hexchar2dec(nibble2 "${nibble2}")

    MATH(EXPR nibble "${nibble1} ^ ${nibble2}")
    dec2hexchar(nibble ${nibble})
    set(${result} "${${result}}${nibble}")
    MATH(EXPR index "${index} + 1")
  endwhile()
endmacro()
