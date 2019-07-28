# Shift Library Collection

This repository hosts a collection of various libraries and tools that have been created in the last couple of years.

## Content

* [shift.core](shift/core/doc/core.md): various generic algorithms and data structures.
* [shift.platform](shift/platform/doc/platform.md): platform specific functionality.
* [shift.log](shift/log/doc/log.md): a powerful log library.
* [shift.math](shift/math/doc/math.md): a math library optimized for use in 3D applications.
* [shift.parser.json](shift/parser.json/doc/parser.json.md): yet another JSON parser.
* [shift.parser.xml](shift/parser.xml/doc/parser.xml.md): yet another XML parser.
* [shift.task](shift/task/doc/task.md): a task processing library using coroutines.
* [shift.serialization](shift/serialization/doc/serialization.md): a flexible serialization library.
* [shift.network](shift/network/doc/network.md): a network abstraction library.
* [shift.service](shift/service/doc/service.md): library for distributed services.
* [shift.application](shift/application/doc/application.md): a minimalistic library to initialize common components.
* [shift.proto](shift/proto/doc/proto.md): grammar and parser of the shift proto language.
* [shift.protogen](shift/protogen/doc/protogen.md): code generator for shift.proto.
* [shift.parser.proto](shift/parser.proto/doc/parser.proto.md): experimental version 2 of shift.proto.
* [shift.tools.protogen](shift/tools.protogen/doc/tools.protogen.md): experimental version 2 of shift.protogen.
* [shift.resource_db](shift/resource_db/doc/resource_db.md): graphic resource library.
* [shift.rc](shift/rc/doc/rc.md): resource compiler core library.
* [shift.tools.rc](shift/tools.rc/doc/tools.rc.md): resource compiler frontend application.
* [shift.tools.vk2cpp](shift/tools.vk2cpp/doc/tools.vk2cpp.md): a tool to translate the Vulkan specification XML file into a C++ header plus MSVS and QtCreator code snippets.
* [shift.render.vk](shift/render.vk/doc/render.vk.md): a Vulkan renderer.

## Dependencies

The code is regularly tested with the following compilers:

* [GCC 9.1.0](https://gcc.gnu.org/)
* [Clang 8.0.0](https://clang.llvm.org/) (see [Using Clang](#using-clang))
* [MSVC 16.1](https://visualstudio.microsoft.com/)

The following external libraries are required:

* [zlib](https://zlib.net/) 1.2.11
* [xz](https://tukaani.org/xz/) 5.2.4
* [libtiff](http://www.simplesystems.org/libtiff/) 4.0.9
    * [lcms](http://www.littlecms.com/) 2.9
* [libpng](http://www.libpng.org/pub/png/libpng.html) 1.6.35
* [libjpeg](https://www.ijg.org/) 9c
* [GSL](https://github.com/Microsoft/GSL)
* [Boost](https://www.boost.org/) 1.70
* [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) tested with revision cc7679e
* [ShaderC](https://github.com/google/shaderc) tested with revision v2019.0
    * [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools)
    * [glslang](https://github.com/KhronosGroup/glslang)
* [Vulkan SDK](https://vulkan.lunarg.com/) 1.1.82.0
* [GLFW](https://www.glfw.org/) 3.2.1
* [Qt](https://www.qt.io/download) 5.11
* [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) 7.0
    * Used to format generated code.

The following external libraries are optional:

* [Google Breakpad](https://github.com/google/breakpad) tested with revision 1459e5d
    * [GYP](https://gyp.gsrc.io/) is needed to build Breakpad
    * [Python](https://www.python.org/) 2.7 is needed for running GYP

For Windows you can try [3rdparty build scripts](3rdparty/packages/README.md) supplied with this repository.

## Using Clang

When using Clang with GCC's standard library libstdc++ you need the following patch to work around a long standing bug in Clang:

```
--- /usr/include/c++/9.1.0/variant.orig	2019-06-19 22:08:51.393999303 +0200
+++ /usr/include/c++/9.1.0/variant	2019-06-19 22:12:42.959869467 +0200
@@ -1217,7 +1217,11 @@
 
   template<typename... _Types>
     class variant
+#if defined(__clang__)
+    : public __detail::__variant::_Variant_base<_Types...>,
+#else
     : private __detail::__variant::_Variant_base<_Types...>,
+#endif
       private _Enable_default_constructor<
 	__detail::__variant::_Traits<_Types...>::_S_default_ctor,
 	  variant<_Types...>>,
@@ -1555,8 +1559,10 @@
     private:
 #endif
 
+#if !defined(__clang__)
       template<size_t _Np, typename _Vp>
 	friend constexpr decltype(auto) __detail::__variant::__get(_Vp&& __v);
+#endif
 
       template<typename _Vp>
 	friend void* __detail::__variant::__get_storage(_Vp&& __v);
```

