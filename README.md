# Shift Library Collection

This repository hosts a collection of various libraries that have been created in the last couple of years.

## Content

* [shift.core](shift/core/doc/core.md): various generic algorithms and data structures.
* [shift.platform](shift/platform/doc/platform.md): platform specific functionality.
* [shift.log](shift/log/doc/log.md): a powerful log library.
* [shift.math](shift/math/doc/math.md): a math library optimized for use in 3D applications.
* [shift.parser.json](shift/parser.json/doc/parser.json.md): yet another JSON parser.
* [shift.task](shift/task/doc/task.md): a task processing library using coroutines.
* [shift.serialization](shift/serialization/doc/serialization.md): a flexible serialization library.
* [shift.network](shift/network/doc/network.md): a network abstraction library.
* [shift.service](shift/service/doc/service.md): library for distributed services.
* [shift.proto](shift/proto/doc/proto.md): grammar and parser of the shift proto language.
* [shift.protogen](shift/protogen/doc/protogen.md): code generator for shift.proto.
* [shift.parser.proto](shift/parser.proto/doc/parser.proto.md): experimental version 2 of shift.proto.
* [shift.tools.protogen](shift/tools.protogen/doc/tools.protogen.md): experimental version 2 of shift.protogen.
* [shift.rc](shift/rc/doc/rc.md): resource compiler core library.
* [shift.tools.rc](shift/tools.rc/doc/tools.rc.md): resource compiler frontend application.
* [shift.tools.vk2cpp](shift/tools.vk2cpp/doc/tools.vk2cpp.md): a tool to translate the Vulkan specification XML file into a C++ header plus MSVS and QtCreator code snippets.

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
