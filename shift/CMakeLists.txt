# Platform specific threading library.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Several OS specific libraries.
find_package(OSLibs REQUIRED)

find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(Breakpad REQUIRED)
if (NOT NO_VULKAN)
  find_package(GLFW3)
  find_package(Vulkan)

  find_package(PNG)
  find_package(JPEG)
  find_package(TIFF)
  find_package(ShaderC)
  find_package(SPIRVCross)
endif()

# Boost libraries.
find_package(Boost ${Boost_VERSION} REQUIRED
  COMPONENTS
    unit_test_framework
    ${BOOST_STACKTRACE_STATIC_LIB}
    coroutine
    filesystem
    program_options
    iostreams
    regex
    serialization
    date_time
    context
    locale
    system
    thread
    chrono
    atomic
  )

set(SHIFT_VERSION_MAJOR 2)
set(SHIFT_VERSION_MINOR 0)

add_subdirectory(platform)
add_subdirectory(core)
add_subdirectory(math)
add_subdirectory(log)
add_subdirectory(crypto)
add_subdirectory(task)
add_subdirectory(parser.json)
add_subdirectory(parser.xml)
add_subdirectory(serialization)
add_subdirectory(serialization2)
add_subdirectory(network)
add_subdirectory(service)
add_subdirectory(livedebug)
add_subdirectory(application)

add_subdirectory(proto)
add_subdirectory(protogen)

add_subdirectory(parser.proto)
add_subdirectory(tools.protogen)

if (NOT NO_VULKAN)
  add_subdirectory(scene)
  add_subdirectory(resource)
  add_subdirectory(render.vk)
  add_subdirectory(tools.rc)
endif()

