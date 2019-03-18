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
set SOURCE_PACKAGE=boost_1_69_0
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  "%MINGW64%\..\..\mingw64\bin\curl.exe" -L -J -O "https://boxie.eu/3rdparty/!SOURCE_PACKAGE!.zip"
)
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  echo Error: Source package !SOURCE_PACKAGE!.zip is missing.
  goto end
)

where /q python.exe
if %ERRORLEVEL% NEQ 0 (
  echo Error: You need to have Python in your system path to allow Boost to
  echo automatically detect it's installation folder.
  goto end
)

"%MINGW64%\unzip" .\!SOURCE_PACKAGE!.zip
if %ERRORLEVEL% NEQ 0 goto end
pushd %CD%
cd !SOURCE_PACKAGE!

call bootstrap.bat

"%MINGW64%\patch" -p1 < ..\!SOURCE_PACKAGE!-gil-tiff-reader.patch
IF NOT ERRORLEVEL 0 goto end

set ZLIB_NAME=zlib
set ZLIB_LIBRARY_PATH=!BUILD_PREFIX!\lib
set ZLIB_INCLUDE=!BUILD_PREFIX!\include

if /i "%1"=="msvc15" (
  b2 --build-type=minimal ^
    --without-mpi ^
    --without-signals ^
    toolset=msvc-14.1 ^
    architecture=x86 ^
    address-model=%BUILD_BITS% ^
    link=static ^
	define=BOOST_USE_WINAPI_VERSION=0x0A00 ^
    -sZLIB_BINARY=zlib ^
    -sZLIB_INCLUDE=!BUILD_PREFIX!\include ^
    -sZLIB_LIBPATH=!BUILD_PREFIX!\lib ^
    -sBZIP2_BINARY=libbz2 ^
    -sBZIP2_INCLUDE=!BUILD_PREFIX!\include ^
    -sBZIP2_LIBPATH=!BUILD_PREFIX!\lib ^
    -sICU_PATH=!BUILD_PREFIX! ^
    --prefix=!BUILD_PREFIX! install -j8
) else (
  echo ToDo: Add boost build command for %1.
)

:end
popd
if EXIST ".\!SOURCE_PACKAGE!" "%MINGW64%\rm" -rf .\!SOURCE_PACKAGE!
