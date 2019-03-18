@echo off
REM Copyright (C) 2012-2019 Christian Spanier
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
set SOURCE_PACKAGE=SPIRV-Cross
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  "%MINGW64%\..\..\mingw64\bin\curl.exe" -L -J -O "https://boxie.eu/3rdparty/!SOURCE_PACKAGE!.zip"
)
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  echo Error: Source package !SOURCE_PACKAGE!.zip is missing.
  goto end
)

"%MINGW64%\unzip" .\!SOURCE_PACKAGE!.zip
if %ERRORLEVEL% NEQ 0 goto end
pushd %CD%
cd !SOURCE_PACKAGE!

"%MINGW64%\patch" -p1 < ..\!SOURCE_PACKAGE!-debug.patch
IF NOT ERRORLEVEL 0 goto end

REM Build library
mkdir build
cd build
cmake -G "%CMAKE_GENERATOR%" ..
msbuild SPIRV-Cross.sln /t:Rebuild /p:Configuration=Debug;Platform=%BUILD_PLATFORM%
msbuild SPIRV-Cross.sln /t:Rebuild /p:Configuration=Release;Platform=%BUILD_PLATFORM%
cd ..

REM ToDo: Copy 'glslang.pdb'
REM ToDo: Copy 'HLSL.pdb'
REM ToDo: Copy 'OGLCompiler.pdb'
REM ToDo: Copy 'OSDependent.pdb'
REM ToDo: Copy 'shaderc.pdb'
REM ToDo: Copy 'shaderc_util.pdb'
REM ToDo: Copy 'spirv-cross-core.pdb'
REM ToDo: Copy 'SPIRV-Tools-opt.pdb'
REM ToDo: Copy 'SPIRV-Tools.pdb'
REM ToDo: Copy 'SPIRV.pdb'
"%MINGW64%\cp" -v build/Debug/spirv-cross*.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -v build/Release/spirv-cross*.lib !BUILD_PREFIX!/lib
"%MINGW64%\cp" -rv include/* !BUILD_PREFIX!/include

popd
"%MINGW64%\rm" -rf ./!SOURCE_PACKAGE!

:end
