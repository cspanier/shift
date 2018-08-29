if (NOT "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
  message(FATAL_ERROR "CMAKE_HOST_SYSTEM_NAME must be set to 'Windows'.")
endif()

set(WindowsSDKVersion $ENV{WindowsSDKVersion} CACHE PATH
  "Path to the Windows SDK to use.")
if ("$ENV{WindowsSDKVersion}" STREQUAL "" AND
  "${WindowsSDKVersion}" STREQUAL "")
  message(FATAL_ERROR "Cannot find environment variable WindowsSDKVersion.\n"
    "Please call this script from a command prompt where all build environment "
    "variables for the given compiler toolchain are properly set.")
endif()
# Remove any backslashes from the value of WindowsSDKVersion.
string(REGEX REPLACE "\\\\" "" WINSDK_VERSION ${WindowsSDKVersion})
message(STATUS "Using Windows SDK version ${WINSDK_VERSION}")

# Don't set CMAKE_HOST_SYSTEM_PROCESSOR manually to support both 32 and 64 bit
# host systems.
# set(CMAKE_HOST_SYSTEM_PROCESSOR AMD64)
set(CMAKE_HOST_SYSTEM_NAME Windows)
set(CMAKE_HOST_SYSTEM_VERSION ${WINSDK_VERSION})

set(CMAKE_SYSTEM_PROCESSOR AMD64)

# Don't set this variable manually, because otherwise CMake will enable
# crosscompilation (regardless of the fact that it is equal to
# CMAKE_HOST_SYSTEM_NAME).
# set(CMAKE_SYSTEM_NAME Windows)

# Setting CMAKE_SYSTEM_VERSION without CMAKE_SYSTEM_NAME is explicitely allowed
# and won't enable crosscompilation as long as the executable will run on the
# host system as well.
set(CMAKE_SYSTEM_VERSION ${WINSDK_VERSION})
