cmake_minimum_required(VERSION 3.12)

option(SHIFT_DEBUG
  "Prints the values of several variables to console to aid in debugging.")

# Specifies a CMake file that overrides platform information. It is loaded after
# CMake’s builtin compiler and platform information modules have been loaded but
# before the information is used. The file may set platform information
# variables to override CMake’s defaults.
set(CMAKE_USER_MAKE_RULES_OVERRIDE
  "${CMAKE_SOURCE_DIR}/cmake/ShiftCompilerFlagOverrides.cmake")

# Enable output of compile commands during generation. CMake then generates a
# compile_commands.json file containing the exact compiler calls for all
# translation units of the project in machine-readable form.
# Note: This only works for Makefile and Ninja generators.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if((CMAKE_GENERATOR MATCHES "Visual Studio") AND
  (CMAKE_GENERATOR_TOOLSET STREQUAL ""))
  message(WARNING "Visual Studio generators use the x86 host compiler by "
                  "default, even for 64-bit targets. This can result in linker "
                  "instability and out of memory errors. To use the 64-bit "
                  "host compiler, pass -Thost=x64 on the CMake command line.")
endif()

# Enable C/C++ support for CMake.
project("shift2" C CXX)

################################################################################

if(NOT CMAKE_TOOLCHAIN_FILE)
  # If you encounter this error it means that you forgot to pass
  # -DCMAKE_TOOLCHAIN_FILE=<path-to-toolchain-file> to CMake.
  message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE is empty.\n"
    "Please define this CMake variable to something like "
    "\"cmake/Toolchain-[processor]-[system name]-[compiler].cmake\"")
endif()

# We define our own target processor variable because CMAKE_SYSTEM_PROCESSOR is
# not fully portable and relies on system specific values.
if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "^(x86_64|AMD64)$")
  set(SHIFT_SYSTEM_PROCESSOR x86_64)
else()
  message(FATAL_ERROR "CMAKE_SYSTEM_PROCESSOR has an unsupported value of "
    "\"${CMAKE_SYSTEM_PROCESSOR}\".")
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
   NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  message(FATAL_ERROR "Unsupported platform \"${CMAKE_SYSTEM_NAME}\".")
endif()

# Build a short compiler name and version used in binary file names.
if(NOT SHIFT_COMPILER_ACRONYM)
  if(MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.10)
    set(compiler_acronym "vc141")
  elseif(MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.0)
    set(compiler_acronym "vc140")
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
      OUTPUT_VARIABLE GCC_VERSION)
    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
    list(GET GCC_VERSION_COMPONENTS 0 GCC_VERSION_MAJOR)
    list(GET GCC_VERSION_COMPONENTS 1 GCC_VERSION_MINOR)
    set(compiler_acronym "gcc${GCC_VERSION_MAJOR}${GCC_VERSION_MINOR}")
  elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version
      OUTPUT_VARIABLE CLANG_VERSION)
    string(REGEX MATCHALL "[0-9]+" CLANG_VERSION_COMPONENTS ${CLANG_VERSION})
    list(GET CLANG_VERSION_COMPONENTS 0 CLANG_VERSION_MAJOR)
    list(GET CLANG_VERSION_COMPONENTS 1 CLANG_VERSION_MINOR)
    set(compiler_acronym "clang${CLANG_VERSION_MAJOR}${CLANG_VERSION_MINOR}")
  else()
    message(FATAL_ERROR "Unknown or unsupported compiler.")
  endif()
  set(SHIFT_COMPILER_ACRONYM ${compiler_acronym} CACHE INTERNAL
    "Compiler short name and version.")
endif()

################################################################################

include("${CMAKE_SOURCE_DIR}/cmake/ShiftUtilityMacros.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/ShiftUtilityDebugVar.cmake")
# include("${CMAKE_SOURCE_DIR}/cmake/ShiftFindLibrary.cmake")
# include("${CMAKE_SOURCE_DIR}/cmake/ShiftFindHeader.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/ShiftAddExecutable.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/ShiftAddLibrary.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/ShiftAddTests.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/ShiftAddCSharpProject.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/ShiftAddProtogen.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/ShiftAddDocumentation.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/ShiftGitVersion.cmake")

# Enable testing in this folder and below.
enable_testing()

# Enable use of virtual folders in Microsoft Visual Studio project generators.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define a custom directory where to look for modules to be loaded by the
# include() or find_package() commands before checking the default modules that
# come with CMake.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

debug_var(CMAKE_PREFIX_PATH)
debug_var(CMAKE_TOOLCHAIN_FILE)
debug_var(CMAKE_CROSSCOMPILING)

# Override the default installation location unless the user manually specified
# a different path.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/production" CACHE PATH
    "Install directory used by install()." FORCE)
endif()
debug_var(CMAKE_INSTALL_PREFIX)

################################################################################
# Boost
set(Boost_VERSION "1.68")
if(NOT BOOST_ROOT)
  # Boost passes NO_MODULE to find_package which makes it ignore
  # CMAKE_FIND_ROOT_PATH.
  set(BOOST_ROOT "${CMAKE_FIND_ROOT_PATH}")
endif()
set(Boost_DEBUG OFF)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ADDITIONAL_VERSIONS "${Boost_VERSION}" "${Boost_VERSION}.0")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # # ToDo: Is this still needed with the latest version of cmake?
  # if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.10)
  #   set(Boost_COMPILER "-vc141")
  #   message(STATUS "Fixing Boost_COMPILER=${Boost_COMPILER}")
  # elseif (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19)
  #   set(Boost_COMPILER "-vc140")
  #   message(STATUS "Fixing Boost_COMPILER=${Boost_COMPILER}")
  # elseif(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18)
  #   set(Boost_COMPILER "-vc120")
  #   message(STATUS "Fixing Boost_COMPILER=${Boost_COMPILER}")
  # endif()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  # if (MSVC)
  #   set(Boost_COMPILER "-vc141")
  #   message(STATUS "Fixing Boost_COMPILER=${Boost_COMPILER}")
  # else()
  #   # ToDo: Why isn't clang automatically detected?
  #   set(Boost_COMPILER "-clang${CLANG_VERSION_MAJOR}${CLANG_VERSION_MINOR}")
  # endif()
endif()
if("${SHIFT_SYSTEM_PROCESSOR}" MATCHES "^ARM.*")
  set(BOOST_THREAD "thread")
endif()

################################################################################

set(SHIFT_GLOBAL_DEFINITIONS
  # Disable automatic linking to Boost libraries (which is handled by CMake
  # instead).
  "BOOST_ALL_NO_LIB"
  
  # This macro is useful for when updating the compiler to a version not yet
  # known to Boost.
  # "BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE"

  # Disable deprecated methods und functions in Boost Filesystem.
  "BOOST_FILESYSTEM_NO_DEPRECATED"

  # Prefer use of Boost Coroutine version 2 in various Boost libraries.
  "BOOST_COROUTINES_V2"

  # Boost Asio (1.68) still makes use of Boost coroutine version 1, which is
  # deprecated.
  "BOOST_COROUTINES_NO_DEPRECATION_WARNING"
  )

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND SHIFT_GLOBAL_DEFINITIONS
    "SHIFT_PLATFORM_LINUX"

    # Rarely used by external code (e.g. AMD Tootle)
    "_LINUX"

    # Compilers supporting auto-tss cleanup may define BOOST_THREAD_USE_LIB.
    # ToDo: This should have no effect because we disabled automatic linking
    # using BOOST_ALL_NO_LIB above.
    "BOOST_THREAD_USE_LIB=1"

    # Enable Valgrind support for Boost Context and Boost Coroutine (version 1).
    # This is required to identify stack regions as such, so they are correctly
    # handled by Valgrind.
    "BOOST_USE_VALGRIND=1"

    # Enable custom assert handlers defined in shift/platform/private/assert.cpp
    "BOOST_ENABLE_ASSERT_DEBUG_HANDLER"
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  list(APPEND SHIFT_GLOBAL_DEFINITIONS
    "SHIFT_PLATFORM_WINDOWS"
    "WIN32"
    "_WINDOWS"
    # We need to define both WINVER and _WIN32_WINNT.
    "WINVER=0x0A00"  # Windows 10
    "_WIN32_WINNT=0x0A00"  # Windows 10
    "_CONSOLE"

    # Drastically reduce the number of implicite includes introduced by
    # Windows.h to improve compile speed.
    "WIN32_LEAN_AND_MEAN"

    # Prevent Windows.h from defining macros min and max, which collide with
    # std::min and std::max.
    "NOMINMAX"

    # Enables math defined which are not in Standard C/C++, but were introduced
    # by Microsoft long ago. Some code today depends on them to be defined.
    # This includes M_PI, M_PI_2, M_SQRT2, M_E, ...
    # "_USE_MATH_DEFINES"

    # See type_traits header:
    # You've instantiated std::aligned_storage<Len, Align> with an extended alignment (in other 
    # words, Align > alignof(max_align_t)). Before VS 2017 15.8, the member type would 
    # non-conformingly have an alignment of only alignof(max_align_t). VS 2017 15.8 was fixed to 
    # handle this correctly, but the fix inherently changes layout and breaks binary compatibility 
    # (*only* for uses of aligned_storage with extended alignments). 
    # Please define either 
    # (1) _ENABLE_EXTENDED_ALIGNED_STORAGE to acknowledge that you understand this message and 
    # that you actually want a type with an extended alignment, or 
    # (2) _DISABLE_EXTENDED_ALIGNED_STORAGE to silence this message and get the old non-conformant 
    # behavior.
    "_ENABLE_EXTENDED_ALIGNED_STORAGE"

    # This macro enables use of a pre-compiled Boost stacktrace library. Without
    # Boost stacktrace is a header-only library, but requires additional system
    # dependencies.
    # "BOOST_STACKTRACE_LINK"

    # Without BOOST_STACKTRACE_LINK being defined this selects the stacktrace
    # implementation to use.
    "BOOST_STACKTRACE_USE_WINDBG"

    # ToDo: Add info about this macro and why it is used.
    # "BOOST_USE_WINDOWS_H"
  )

    # Add processor macros as required external headers.
  if("${SHIFT_SYSTEM_PROCESSOR}" STREQUAL "x86_32")
    list(APPEND SHIFT_GLOBAL_DEFINITIONS
      # ToDo: Who needs this?
      "_WIN32"
      # Used by winnt.h
      "_X86_"
    )
  elseif("${SHIFT_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    list(APPEND SHIFT_GLOBAL_DEFINITIONS
      # ToDo: Who needs this?
      "WIN64"
      # ToDo: Who needs this?
      "_WIN64"
      # Used by winnt.h
      "_AMD64_"
    )
  endif()

  # If BOOST_STACKTRACE_LINK is defined above, we need to manually link the
  # appropriate static library shipping with Boost.
  # set(BOOST_STACKTRACE_STATIC_LIB "stacktrace_windbg")
endif()

################################################################################

add_subdirectory(shift)
add_subdirectory(shift_net)
