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
set SOURCE_PACKAGE=zstd-be3bd70
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  echo Error: Source package !SOURCE_PACKAGE!.zip is missing.
  goto end
)

%GNUWIN32%\unzip .\!SOURCE_PACKAGE!.zip
if %ERRORLEVEL% NEQ 0 goto end
pushd %CD%
cd !SOURCE_PACKAGE!\build\cmake

mkdir build
cd build
cmake -G "%CMAKE_GENERATOR%" .. ^
  -DZSTD_BUILD_CONTRIB=OFF ^
  -DZSTD_BUILD_TESTS=OFF ^
  -DZSTD_BUILD_SHARED=OFF ^
  -DZSTD_BUILD_STATIC=ON

msbuild lib\libzstd_static.vcxproj /t:Rebuild /p:Configuration=Debug;Platform=%BUILD_PLATFORM%
msbuild lib\libzstd_static.vcxproj /t:Rebuild /p:Configuration=Release;Platform=%BUILD_PLATFORM%

%GNUWIN32%\cp -v lib\Debug\zstd_static.lib %BUILD_PREFIX%\lib\zstd_static_d.lib
%GNUWIN32%\cp -v lib\Release\zstd_static.lib %BUILD_PREFIX%\lib
%GNUWIN32%\cp -v ..\..\..\lib\zstd.h %BUILD_PREFIX%\include

:end
popd
if EXIST ".\!SOURCE_PACKAGE!" %GNUWIN32%\rm -rf .\!SOURCE_PACKAGE!
