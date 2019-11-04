if (NOT "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
  message(FATAL_ERROR "CMAKE_HOST_SYSTEM_NAME must be set to 'Linux'.")
endif()

# Don't set CMAKE_HOST_SYSTEM_PROCESSOR manually to support both 32 and 64 bit
# host systems.
# set(CMAKE_HOST_SYSTEM_PROCESSOR x86_64)
set(CMAKE_HOST_SYSTEM_NAME Linux)

set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Don't set this variable manually, because otherwise CMake will enable
# crosscompilation (regardless of the fact that it is equal to
# CMAKE_HOST_SYSTEM_NAME).
# set(CMAKE_SYSTEM_NAME Linux)

set(SHIFT_PLATFORM_LINUX TRUE)

# Select standard library to use. This may be either Clang's libc++ or GCC's libstdc++.
set(SHIFT_CLANG_STDLIB libc++)
