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

REM This batch script is used to automatically build a bunch of 3rd party
REM libraries for Windows using Microsoft Visual Studio.
REM Only 64bit builds are fully supported right now and 32bit builds may fail
REM for some libraries.

goto start

REM ############################################################################

:help
echo Usage: %0 (msvc15) (x86_32^|x86_64) (package^|all)
echo package may be one of the available package build batch files' names
echo without file extension. Use 'all' to build all packages.
echo.
echo Note that some packages don't support all compiler or platform settings.
echo However, msvc15 and x86_64 should always be implemented.
echo.
echo You may set the environment variable BUILD_PREFIX to overwrite the target
echo path where to install libraries to.
goto end

REM ############################################################################

:start
setlocal EnableDelayedExpansion

pushd
cd %~dp0

if "!MINGW64!"=="" (
  set MINGW64=C:\Program Files\Git\usr\bin
  echo Automatically setting MINGW64 installation folder to !MINGW64!.
)
if NOT EXIST "%MINGW64%\cp.exe" (
  echo Error: Missing cp command!
  goto end
)
if NOT EXIST "%MINGW64%\rm.exe" (
  echo Error: Missing rm command!
  goto end
)
if NOT EXIST "%MINGW64%\mkdir.exe" (
  echo Error: Missing mkdir command!
  goto end
)
if NOT EXIST "%MINGW64%\patch.exe" (
  echo Error: Missing patch command!
  goto end
)
if NOT EXIST "%MINGW64%\unzip.exe" (
  echo Error: Missing unzip command!
  goto end
)
if NOT EXIST "%MINGW64%\tee.exe" (
  echo Error: Missing tee command!
  goto end
)
if NOT EXIST "%MINGW64%\sed.exe" (
  echo Error: Missing sed command!
  goto end
)

REM ############################################################################

if "%1"=="" goto help
if "%1"=="/?" goto help
if "%1"=="help" goto help
if "%1"=="-help" goto help
if "%1"=="--help" goto help

if /i "%2"=="x86_32" (
  set BUILD_BITS=32
  set BUILD_PLATFORM=Win32
  set BUILD_OPTIONAL_PLATFORM=
) else if /i "%2"=="x86_64" (
  set BUILD_BITS=64
  set BUILD_PLATFORM=x64
  set BUILD_OPTIONAL_PLATFORM=x64/
) else (
  echo Error: Illegal parameter %2
  goto help
)

if /i "%1"=="msvc15" (
  set BUILD_TOOLSET=vc141
  if /i "%BUILD_BITS%"=="32" (
    set CMAKE_GENERATOR=Visual Studio 15 2017
  ) else if /i "%BUILD_BITS%"=="64" (
    set CMAKE_GENERATOR=Visual Studio 15 2017 Win64
  )
) else (
  echo Error: Illegal parameter %1
  goto help
)

where cl.exe 2>nul >nul
if ERRORLEVEL 1 (
  pushd %CD%
  if /i "%1"=="msvc15" (
    if /i "%BUILD_BITS%"=="32" (
      call "c:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86
    ) else if /i "%BUILD_BITS%"=="64" (
      call "c:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" amd64
    )
  )
  popd
)

if "!BUILD_PREFIX!"=="" (
  set BUILD_PREFIX=%CD%\..\%2-windows-!BUILD_TOOLSET!
  echo Automatically set BUILD_PREFIX = !BUILD_PREFIX!
) else (
  echo Using externally set value for BUILD_PREFIX = !BUILD_PREFIX!
)
if NOT EXIST "!BUILD_PREFIX!" (
  "%MINGW64%\mkdir" -p !BUILD_PREFIX!
)
if NOT EXIST "!BUILD_PREFIX!\bin" (
  "%MINGW64%\mkdir" -p !BUILD_PREFIX!\bin
)
if NOT EXIST "!BUILD_PREFIX!\include" (
  "%MINGW64%\mkdir" -p !BUILD_PREFIX!\include
)
if NOT EXIST "!BUILD_PREFIX!\lib" (
  "%MINGW64%\mkdir" -p !BUILD_PREFIX!\lib
)

set BUILD_CALL=1

if /i "%3"=="" (
  echo Error: Missing package parameter.
  goto help
) else if /i "%3"=="build" (
  echo Error: Illegal package parameter.
  goto help
)
if NOT EXIST ".\%3.cmd" (
  echo Error: There is no package build script named %3.cmd.
  goto help
)

call ".\%3.cmd" %1 %2 2>&1 | "%MINGW64%\tee" ".\%3.log"

:end
popd
