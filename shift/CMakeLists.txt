# Platform specific threading library.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Several OS specific libraries.
find_package(OSLibs REQUIRED)

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

find_package(Breakpad)

set(SHIFT_VERSION_MAJOR 2)
set(SHIFT_VERSION_MINOR 0)

add_subdirectory(platform)
add_subdirectory(core)
add_subdirectory(math)
add_subdirectory(log)
add_subdirectory(application)
add_subdirectory(crypto)
add_subdirectory(task)
add_subdirectory(parser.json)
add_subdirectory(parser.xml)
add_subdirectory(serialization)
add_subdirectory(serialization2)
add_subdirectory(parser.proto)
add_subdirectory(tools.protogen)
add_subdirectory(proto)
add_subdirectory(protogen)
add_subdirectory(network)
add_subdirectory(service)
