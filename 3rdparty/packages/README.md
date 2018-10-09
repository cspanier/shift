# 3rdparty

## Introduction

While most Linux distributions provide nice package managers, building C++ libraries for Windows manually is often a time consuming process. Some reasons for this are

* missing standards for installation locations of header files and binaries.
* missing naming conventions for filenames (e.g. lack of version numbers in binaries).
* lack of common tools usually found on Linux systems
* diversity of external executables used during each libary's build tool chain (msbuild, nmake, ninja, jom, cmake, gyp, ...)

Things get worse when you want to upgrade a library to a new version, or need to rebuild all 3rdparty libraries due to a critical bugfix in your compiler.
You then have to repeat the whole process of manual building, which is not fun.

This folder provides a collection of build scripts for a variety of C++ libraries targeting Microsoft Windows.
The scripts try to unify installation for each and every library to a common Unix like structure:

```
x86_64-windows-vc141  # When building for 64bit Windows using MSVC15 (2017)
  +- bin              # DLLs and PDB files
  +- include          # Header files
  +- lib              # Static libraries
```

This structure enables easy integration into your application's build system (e.g. by using CMAKE_PREFIX_PATH with CMake).

## Usage

There is only a single script you need to call named `build.cmd`:

```
build.cmd (msvc15) (x86_64) (<package_name>|all)
```

In case you wonder about the first and second argument: There used to be other MSVC versions and 32 bit support. These arguments might be extended in future releases.

## Windows Dependencies

* `Git for Windows`  
  The batch scripts used to depend on `GnuWin32` binaries. I recently changed this `MinGW64`, which is shipped as part of `Git for Windows` (you most likely installed this already).
* Microsoft Visual Studio 2017
* Python 2.7 (optional)
  Some packages require Python 2.7 to be installed (e.g. Boost Python, or Google Breakpad).
* GYP (optional)
  Google Breakpad requires GYP to be installed and in system PATH.
