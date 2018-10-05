# Shift Library Collection

This repository hosts a collection of various libraries that have been created in the last couple of years.

## Content

* shift.core: various generic algorithms and data structures.
* shift.platform: platform specific functionality.
* shift.log: a powerful log library.
* shift.math: a math library optimized for use in 3D applications.
* shift.task: a task processing library using coroutines.
* shift.serialization: a datastructure serialization library.
* shift.network: a network abstraction library.
* shift.service: library for distributed services.
* shift.proto: grammar and parser of the shift proto language.
* shift.protogen: code generator for shift.proto.
* shift.parser.proto: experimental version 2 of shift.proto.
* shift.tools.protogen: experimental version 2 of shift.protogen.
* [shift.rc](shift/rc/doc/rc.md): resource compiler core library.
* [shift.tools.rc](shift/tools.rc/doc/tools.rc.md): resource compiler frontend application.
* shift.tools.vk2cpp: a tool to translate the Vulkan specification XML file into a C++ header plus MSVS and QtCreator code snippets.

## Dependencies

* [Boost](https://www.boost.org/) 1.68
* [Google Breakpad](https://github.com/google/breakpad) tested with revision 1459e5d
  * [GYP](https://gyp.gsrc.io/) is needed to build Breakpad
  * [Python](https://www.python.org/) 2.7 is needed for running GYP
* [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) tested with revision cc7679e
* [ShaderC](https://github.com/google/shaderc) tested with revision 30af9f9
* [Vulkan SDK](https://vulkan.lunarg.com/) 1.1.82.0
* [GLFW](https://www.glfw.org/) 3.2.1
* [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) 6.0
  * Used to format generated code.
