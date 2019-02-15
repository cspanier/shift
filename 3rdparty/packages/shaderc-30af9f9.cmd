@echo off
REM Copyright (C) 2012-2018 Christian Spanier
REM
REM Permission is hereby granted, free of charge, to any person obtaining a copy
REM of this software and associated documentation files (the "Software"), to deal
REM in the Software without restriction, including without limitation the rights
REM to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
REM copies of the Software, and to permit persons to whom the Software is
REM furnished to do so, subject to the following conditions:
REM
REM The above copyright notice and this permission notice shall be included in
REM all copies or substantial portions of the Software.
REM
REM THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
REM IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
REM FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
REM AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
REM LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
REM OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
REM SOFTWARE.

REM ############################################################################

setlocal EnableDelayedExpansion
if NOT "%BUILD_CALL%"=="1" (
  echo You need to call build.cmd in order to use this script
  goto end
)
set SOURCE_PACKAGE=shaderc-30af9f9
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  "%MINGW64%\..\..\mingw64\bin\curl.exe" -L -J -O "https://boxie.eu/3rdparty/!SOURCE_PACKAGE!.zip"
)
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  echo Error: Source package !SOURCE_PACKAGE!.zip is missing.
  goto end
)

if NOT EXIST ".\SPIRV-Tools.zip" (
  "%MINGW64%\..\..\mingw64\bin\curl.exe" -L -J -O "https://boxie.eu/3rdparty/SPIRV-Tools.zip"
)
if NOT EXIST ".\SPIRV-Headers.zip" (
  "%MINGW64%\..\..\mingw64\bin\curl.exe" -L -J -O "https://boxie.eu/3rdparty/SPIRV-Headers.zip"
)
if NOT EXIST ".\googletest.zip" (
  "%MINGW64%\..\..\mingw64\bin\curl.exe" -L -J -O "https://boxie.eu/3rdparty/googletest.zip"
)
if NOT EXIST ".\glslang.zip" (
  "%MINGW64%\..\..\mingw64\bin\curl.exe" -L -J -O "https://boxie.eu/3rdparty/glslang.zip"
)

"%MINGW64%\unzip" .\!SOURCE_PACKAGE!.zip
if %ERRORLEVEL% NEQ 0 goto end
pushd %CD%
cd !SOURCE_PACKAGE!

REM Extract external dependencies
cd third_party
"%MINGW64%\unzip" ..\..\SPIRV-Tools.zip
if %ERRORLEVEL% NEQ 0 goto end
"%MINGW64%\unzip" ..\..\SPIRV-Headers.zip
if %ERRORLEVEL% NEQ 0 goto end
"%MINGW64%\unzip" ..\..\googletest.zip
if %ERRORLEVEL% NEQ 0 goto end
"%MINGW64%\unzip" ..\..\glslang.zip
if %ERRORLEVEL% NEQ 0 goto end
cd ..

REM Build library
mkdir build
cd build
cmake -G "%CMAKE_GENERATOR%" .. -DSHADERC_ENABLE_SHARED_CRT=1 -DSHADERC_SKIP_TESTS=1
msbuild shaderc.sln /t:Rebuild /p:Configuration=Debug;Platform=%BUILD_PLATFORM%
msbuild shaderc.sln /t:Rebuild /p:Configuration=Release;Platform=%BUILD_PLATFORM%
cd ..

REM "%MINGW64%\cp" -v build/libshaderc/Debug/shaderc_combined.lib !BUILD_PREFIX!/lib/shaderc_combined_d.lib
REM "%MINGW64%\cp" -v build/libshaderc/Release/shaderc_combined.lib !BUILD_PREFIX!/lib/shaderc_combined.lib

"%MINGW64%\cp" -rv third_party/SPIRV-Tools/include/* !BUILD_PREFIX!/include
"%MINGW64%\cp" -v build/third_party/spirv-tools/source/Debug/SPIRV-Tools.lib !BUILD_PREFIX!/lib/SPIRV-Tools-d.lib
"%MINGW64%\cp" -v build/third_party/spirv-tools/source/Debug/SPIRV-Tools.pdb !BUILD_PREFIX!/lib/SPIRV-Tools-d.pdb
"%MINGW64%\cp" -v build/third_party/spirv-tools/source/Release/SPIRV-Tools.lib !BUILD_PREFIX!/lib/SPIRV-Tools.lib
"%MINGW64%\cp" -v build/third_party/spirv-tools/source/opt/Debug/SPIRV-Tools-opt.lib !BUILD_PREFIX!/lib/SPIRV-Tools-opt-d.lib
"%MINGW64%\cp" -v build/third_party/spirv-tools/source/opt/Debug/SPIRV-Tools-opt.pdb !BUILD_PREFIX!/lib/SPIRV-Tools-opt-d.pdb
"%MINGW64%\cp" -v build/third_party/spirv-tools/source/opt/Release/SPIRV-Tools-opt.lib !BUILD_PREFIX!/lib/SPIRV-Tools-opt.lib

mkdir !BUILD_PREFIX!\include\SPIRV
"%MINGW64%\cp" -v third_party/glslang/SPIRV/*.h !BUILD_PREFIX!/include/SPIRV
"%MINGW64%\cp" -v build/third_party/glslang/SPIRV/Debug/SPIRVd.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -v build/third_party/glslang/SPIRV/Release/SPIRV.lib !BUILD_PREFIX!/lib

mkdir !BUILD_PREFIX!\include\glslang\Include
"%MINGW64%\cp" -v third_party/glslang/glslang/Include/*.h !BUILD_PREFIX!/include/glslang/Include
mkdir !BUILD_PREFIX!\include\glslang\MachineIndependent
"%MINGW64%\cp" -v third_party/glslang/glslang/MachineIndependent/*.h !BUILD_PREFIX!/include/glslang/MachineIndependent
mkdir !BUILD_PREFIX!\include\glslang\Public
"%MINGW64%\cp" -v third_party/glslang/glslang/Public/*.h !BUILD_PREFIX!/include/glslang/Public

"%MINGW64%\cp" -v build/third_party/glslang/OSDependent/Windows/Debug/OSDependentd.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -v build/third_party/glslang/OSDependent/Windows/Release/OSDependent.lib !BUILD_PREFIX!/lib

"%MINGW64%\cp" -v build/third_party/glslang/glslang/Debug/glslangd.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -v build/third_party/glslang/glslang/Release/glslang.lib !BUILD_PREFIX!/lib

"%MINGW64%\cp" -v build/third_party/glslang/hlsl/Debug/HLSLd.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -v build/third_party/glslang/hlsl/Release/HLSL.lib !BUILD_PREFIX!/lib

"%MINGW64%\cp" -v build/third_party/glslang/OGLCompilersDLL/Debug/OGLCompilerd.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -v build/third_party/glslang/OGLCompilersDLL/Release/OGLCompiler.lib !BUILD_PREFIX!/lib

"%MINGW64%\cp" -v build/libshaderc/Debug/shadercd.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -v build/libshaderc/Release/shaderc.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -v build/libshaderc_util/Debug/shaderc_utild.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -v build/libshaderc_util/Release/shaderc_util.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -rv libshaderc/include/* !BUILD_PREFIX!/include

popd
"%MINGW64%\rm" -rf ./!SOURCE_PACKAGE!

:end
