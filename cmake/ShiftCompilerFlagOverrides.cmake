# Override default compiler flags.
# WARNING: All variables in here are automatically cached and cannot be changed.
# You need to manually delete the `CMakeCache.txt` file in your build folder to
# make things happen.

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT MSVC)
  # General compiler flags:
  # -O0 Disable optimizations(required for accurate debugging).
  # -O3 Enable all optimization flags for maximum performance.
  # -Os Enable all optimization flags of -O2 that don't increase the file size.
  # -Wall                      # Enables all the warnings about constructions that some users consider questionable, and that are easy to avoid.
  # -Wextra                    # Enables additional warnings not included in -Wall.
  # -Wno-ignored-attributes    # ToDo: add description
  # -g Add debugging information.
  set(CMAKE_CXX_FLAGS_INIT "-std=c++17 -ftemplate-backtrace-limit=0 -fvisibility=default -fPIC -Wall -ftemplate-depth=512 -Wno-ignored-attributes") # -stdlib=libc++ -fvisibility=hidden -fvisibility-inlines-hidden
  # Temporarily disable redeclared-class-member warning for Boost 1.59
  set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} -Wno-redeclared-class-member")
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g -fno-omit-frame-pointer -DBUILD_CONFIG_DEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-Os -Wextra -fomit-frame-pointer -DBUILD_CONFIG_MINSIZEREL")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -Wextra -fomit-frame-pointer -DBUILD_CONFIG_RELEASE")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-O2 -g -fno-omit-frame-pointer -DBUILD_CONFIG_RELWITHDEBINFO")

  # -fuse-ld=(gold|lld)        # Prefer gold|lld linker over ld
  set(CMAKE_EXE_LINKER_FLAGS_INIT "-lpthread -fuse-ld=lld")
  set(CMAKE_MODULE_LINKER_FLAGS_INIT "")
  set(CMAKE_STATIC_LINKER_FLAGS_INIT "")
  set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")

  # ToDo: This is only needed when using a foreign compiler toolchain which uses
  # a ld-linux-armhf.so.3 different from the target OS installation.
  # if(CMAKE_CROSSCOMPILING)
  #   set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -Wl,--rpath=lib -Wl,--dynamic-linker=lib/ld-linux-armhf.so.3")
  # endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND NOT MSVC)
  # General compiler flags:
  # -O0                        # Disable optimizations(required for accurate debugging).
  # -O3                        # Enable all optimization flags for maximum performance.
  # -Os                        # Enable all optimization flags of -O2 that don't increase the file size.
  # -g                         # Add debugging information.
  # -Wall                      # Enables all the warnings about constructions that some users consider questionable, and that are easy to avoid.
  # -Wextra                    # Enables additional warnings not included in -Wall.
  # -Wno-psabi                 # Disable "parameter passing for argument of type '...' changed in GCC 7.1" warnings
  set(CMAKE_CXX_FLAGS_INIT "-std=c++17 -fvisibility=default -fPIC -Wall -Wno-psabi") # -fvisibility=hidden -fvisibility-inlines-hidden
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g -fno-omit-frame-pointer -DBUILD_CONFIG_DEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-Os -Wextra -fomit-frame-pointer -DBUILD_CONFIG_MINSIZEREL")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -Wextra -fomit-frame-pointer -DBUILD_CONFIG_RELEASE")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-O2 -g -fno-omit-frame-pointer -DBUILD_CONFIG_RELWITHDEBINFO")

  # -fuse-ld=(gold|lld)        # Prefer gold|lld linker over ld
  set(CMAKE_EXE_LINKER_FLAGS_INIT "-lpthread -fuse-ld=gold")
  set(CMAKE_MODULE_LINKER_FLAGS_INIT "")
  set(CMAKE_STATIC_LINKER_FLAGS_INIT "")
  set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=gold")

  # ToDo: This is only needed when using a foreign compiler toolchain which uses
  # a ld-linux-armhf.so.3 different from the target OS installation.
  # if(CMAKE_CROSSCOMPILING)
  #   set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -Wl,--rpath=lib -Wl,--dynamic-linker=lib/ld-linux-armhf.so.3")
  # endif()
elseif(MSVC)
  message(STATUS "Overriding Microsoft Visual C++ Compiler Flags")

  # General compiler flags:
  #   /Z(i|I)   Debug information format
  #     /Zi     Program database
  #     /ZI     Program database for edit and continue
  #   /nologo   Suppress startup banner
  #   /MP       Enable multi-processor compilation
  #   /std:c++(14|17|latest) Select C++ language standard. Using c++latest with Boost requires _HAS_AUTO_PTR_ETC to be defined.
  #   /FS       Force to use MSPDBSRV.EXE
  #   /d2Zi+    Undocumented flag to improve debug information in partially optimized code.
  #             See http://randomascii.wordpress.com/2013/09/11/debugging-optimized-codenew-in-visual-studio-2012/
  #   /Zm<n>    Max memory alloc (% of default)
  #   /permissive- Disable MSVC specifics and try to fully conform to standard C++.
  #   /d2MPX    Enable use of Intel MPX instructions (available on Intel Skylake and beyond).
  set(CMAKE_CXX_FLAGS_INIT "/nologo /FS /Zm256 /MP /std:c++17 /permissive- /D \"_HAS_AUTO_PTR_ETC\"")
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "/Zi /D \"DEBUG\" /D \"BUILD_CONFIG_DEBUG\" /d2MPX")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "/Zi /D \"NDEBUG\" /D \"BUILD_CONFIG_MINSIZEREL\"")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT "/Zi /D \"NDEBUG\" /D \"BUILD_CONFIG_RELEASE\" /analyze")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/Zi /d2Zi+ /D \"NDEBUG\" /D \"BUILD_CONFIG_RELWITHDEBINFO\"")

  # Optimization compiler flags:
  #   /O(d|1|2|x) Optimization:
  #     /Od disabled
  #     /O1 Minimize size
  #     /O2 Maximize speed
  #     /Ox Full optimization
  #   /Ob{n} Inline Function Expansion:
  #     /Ob0 disabled
  #     /Ob1 only __inline
  #     /Ob2 any suitable
  #   /Oi Enable Intrinsic Functions
  #   /O(s|t) Favor size(s) or speed(t)
  #   /Oy Omit frame pointers (prevents debugging)
  #   /GL Whole program optimization
  set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /Oi")
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT} /Od /Ob0")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT} /O1 /Ob2 /Os /Oy /GL")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT} /Ox /Ob2 /Ot /Oy /GL")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT} /O1 /Ob2 /Ot")

  # Code generation compiler flags:
  #   /GF Enable string pooling
  #   /Gm[-] Do (not) enable minimal rebuild
  #   /EHsc Enable C++ exceptions
  #   /EHa Enable C++ exceptions with SEH exceptions
  #   /RTCc Enable runtime checking for conversion to smaller types (debug only)
  #   /RTCs Enable runtime checking for stack frames (debug only)
  #   /RTCu Enable runtime checking for uninitialized variables (debug only)
  #   /RTCsu or /RTC1 Enable both /RTCs and /RTCu (debug only)
  #   /M(T|Td|D|Dd) Runtime library
  #     /MT  Multi-threaded
  #     /MTd Multi-threaded debug
  #     /MD  Multi-threaded DLL
  #     /MDd Multi-threaded DLL debug
  #   /GS[-] Do (not) enable checks for buffer overruns
  #   /Gy Enable function level linking
  #   /arch:(SSE2|AVX) Enable Streaming SIMD Extensions 2 or
  #     Advanced Vector Extensions
  #   /fp:(precise|strict|fast|except[-]) Floating point behavior
  set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /GF /EHa")
  if (NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /Gm-")
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT} /MDd /RTCsu /fp:precise /fp:except")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT} /MD /fp:precise /fp:except")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT} /MD /Gy /fp:fast")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT} /MD /fp:precise /fp:except")
  if (NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT} /GS")
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT} /GS")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT} /GS-")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT} /GS")
  endif()
  if(NOT DISABLE_AVX)
    set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /arch:AVX")
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT}")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT}")
  endif()

  # Language compiler flags:
  #   /Zc:wchar_t Treat wchat_t as built in type
  #   /Zc:forScope[-] Force conformance in for loop scope
  #   /GR[-] Do(not) enable run-time type information
  set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /Zc:wchar_t /Zc:forScope /GR")
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT}")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT}")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT}")

  # Output file compiler flags:
  #   /Fa"path" ASM list location
  #   /Fo"path" Object file location
  #   /Fd"path/file.pdb" Program database file location
  set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT}")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT}")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT}")

  # Advanced compiler flags:
  #   /Gd Calling convention(__cdecl)
  #   /W{n}     Warning level:(0/1/2/3/4/all)
  #   /WX[-]    Do(not) treat warnings as errors(/WX or /WX-)
  #   /wd"{n}" Disable specific warning {n}
  #     4005: macro redefinition
  #     4244: conversion from 'T1' to 'T2', possible loss of data
  #     4267: conversion from 'T1' to 'T2', possible loss of data
  #     4316: object allocated on the heap may not be aligned <n>
  #     4996: 'strxxx': The POSIX name for this item is deprecated. Instead, use the
  #           ISO C++ conformant name: _strxxx. See online help for details.
  set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /Gd /wd4005 /wd4244 /wd4267 /wd4316 /wd4996")
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT} /W3 /WX")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT} /W4")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT} /W4")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT} /W3 /WX")

  # Undocumented flags:
  #   /bigobj Increase Number of Sections in .Obj file
  set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /bigobj")

  message(STATUS "Overriding Microsoft Visual C++ Linker Flags")

  # Linker flags:
  #   /VERSION:"{maj.min.build}" Add a version field into the target executable
  #   /INCREMENTAL[:NO] Enable/disable incremental linking; Incompatible with /OPT:REF.
  #   /NOLOGO Suppress startup banner
  #   /MANIFEST:NO Disables generation of a side-by-side manifest file
  #   /DEBUG Generate debug info
  #   /MAP Generate a map file
  #   /PDB:"{file.pdb}" Generate program database file
  #   /SUBSYSTEM:(CONSOLE|WINDOWS) Selects console/Windows mode for the target executable file
  #   /OPT:REF Eliminate unreferenced functions and data
  #   /OPT:ICF[={n}] Enable COMDAT folding(using {n} iterations)
  #   /LTCG Enable link time code generation
  set(CMAKE_EXE_LINKER_FLAGS_INIT "/NOLOGO /MANIFEST")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "/INCREMENTAL /DEBUG")
  set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT "/INCREMENTAL:NO /DEBUG /OPT:REF")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "/INCREMENTAL:NO /LTCG /OPT:REF /OPT:ICF")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "/INCREMENTAL:NO /DEBUG /OPT:REF")

  if(CMAKE_SIZEOF_VOID_P MATCHES "4")
    set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} /SAFESEH:NO")
  endif()
elseif(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "")
  message(FATAL_ERROR "Unsupported C++ compiler!\nCMAKE_CXX_COMPILER_ID: \"${CMAKE_CXX_COMPILER_ID}\"")
endif()

# Copy linker flags for executables
set(CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT   "${CMAKE_EXE_LINKER_FLAGS_INIT} ${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT}")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} ${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT}")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT}")

# Copy linker flags for static libraries
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT}")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG_INIT "${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT}")
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL_INIT "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT}")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE_INIT "${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT}")
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT}")

# Copy linker flags for shared libraries
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT}")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT "${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT}")
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL_INIT "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE_INIT "${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT}")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT}")

# For single configuration targets, automatically merge compiler flags into
# CMAKE_CXX_FLAGS_INIT.
if(CMAKE_CONFIGURATION_TYPES MATCHES "^${CMAKE_BUILD_TYPE}$" OR
  CMAKE_CONFIGURATION_TYPES MATCHES "^$")
  message(STATUS "Single-configuration target")
  string(TOUPPER "${CMAKE_BUILD_TYPE}" build_type)
  set(CMAKE_CXX_FLAGS_INIT
    "${CMAKE_CXX_FLAGS_INIT} ${CMAKE_CXX_FLAGS_${build_type}_INIT}")
  set(${CMAKE_CXX_FLAGS_${build_type}_INIT} "")
else()
  message(STATUS "Multi-configuration target")
endif()
