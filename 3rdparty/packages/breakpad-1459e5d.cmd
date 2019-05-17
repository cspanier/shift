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
set SOURCE_PACKAGE=breakpad-1459e5d
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  "%MINGW64%\..\..\mingw64\bin\curl.exe" -L -J -O "https://boxie.eu/3rdparty/!SOURCE_PACKAGE!.zip"
)
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  echo Error: Source package !SOURCE_PACKAGE!.zip is missing.
  goto end
)
python --version 2>nul
if %ERRORLEVEL% NEQ 0 (
  echo Error: You need to have Python in your system PATH in order to execute GYP.
  goto end
)

"%MINGW64%\unzip" .\!SOURCE_PACKAGE!.zip
if %ERRORLEVEL% NEQ 0 goto end
pushd %CD%
cd !SOURCE_PACKAGE!\src\client\windows

call ..\..\..\..\gyp\gyp --no-circular-check

REM Patch project files to use the dynamic C++ runtime libraries.
"%MINGW64%\sed" -i.bak "s:<RuntimeLibrary>MultiThreaded</RuntimeLibrary>:<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>:g" common.vcxproj
"%MINGW64%\sed" -i.bak "s:<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>:<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>:g" common.vcxproj
"%MINGW64%\sed" -i.bak "s:<RuntimeLibrary>MultiThreaded</RuntimeLibrary>:<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>:g" crash_generation\crash_generation_client.vcxproj
"%MINGW64%\sed" -i.bak "s:<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>:<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>:g" crash_generation\crash_generation_client.vcxproj
"%MINGW64%\sed" -i.bak "s:<RuntimeLibrary>MultiThreaded</RuntimeLibrary>:<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>:g" crash_generation\crash_generation_server.vcxproj
"%MINGW64%\sed" -i.bak "s:<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>:<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>:g" crash_generation\crash_generation_server.vcxproj
"%MINGW64%\sed" -i.bak "s:<RuntimeLibrary>MultiThreaded</RuntimeLibrary>:<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>:g" handler\exception_handler.vcxproj
"%MINGW64%\sed" -i.bak "s:<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>:<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>:g" handler\exception_handler.vcxproj

msbuild common.vcxproj /t:Rebuild /p:Configuration=Debug;Platform=%BUILD_PLATFORM%
msbuild common.vcxproj /t:Rebuild /p:Configuration=Release;Platform=%BUILD_PLATFORM%
msbuild crash_generation\crash_generation_client.vcxproj /t:Rebuild /p:Configuration=Debug;Platform=%BUILD_PLATFORM%
msbuild crash_generation\crash_generation_client.vcxproj /t:Rebuild /p:Configuration=Release;Platform=%BUILD_PLATFORM%
msbuild crash_generation\crash_generation_server.vcxproj /t:Rebuild /p:Configuration=Debug;Platform=%BUILD_PLATFORM%
msbuild crash_generation\crash_generation_server.vcxproj /t:Rebuild /p:Configuration=Release;Platform=%BUILD_PLATFORM%
msbuild handler\exception_handler.vcxproj /t:Rebuild /p:Configuration=Debug;Platform=%BUILD_PLATFORM%
msbuild handler\exception_handler.vcxproj /t:Rebuild /p:Configuration=Release;Platform=%BUILD_PLATFORM%

cd ..\..

"%MINGW64%\mkdir" -p !BUILD_PREFIX!/lib/breakpad/debug
"%MINGW64%\mkdir" -p !BUILD_PREFIX!/lib/breakpad/release
"%MINGW64%\mkdir" -p !BUILD_PREFIX!/include/breakpad/client/windows/common
"%MINGW64%\mkdir" -p !BUILD_PREFIX!/include/breakpad/client/windows/crash_generation
"%MINGW64%\mkdir" -p !BUILD_PREFIX!/include/breakpad/client/windows/handler
"%MINGW64%\mkdir" -p !BUILD_PREFIX!/include/breakpad/client/windows/sender
"%MINGW64%\mkdir" -p !BUILD_PREFIX!/include/breakpad/common
"%MINGW64%\mkdir" -p !BUILD_PREFIX!/include/breakpad/common/windows
"%MINGW64%\mkdir" -p !BUILD_PREFIX!/include/breakpad/google_breakpad/common
"%MINGW64%\mkdir" -p !BUILD_PREFIX!/include/breakpad/google_breakpad/processor

REM Copy static libraries of debug builds.
"%MINGW64%\cp" -v client/windows/Debug/lib/common.lib !BUILD_PREFIX!/lib/breakpad/debug
"%MINGW64%\cp" -v client/windows/crash_generation/Debug/lib/crash_generation_client.lib !BUILD_PREFIX!/lib/breakpad/debug
"%MINGW64%\cp" -v client/windows/crash_generation/Debug/lib/crash_generation_server.lib !BUILD_PREFIX!/lib/breakpad/debug
"%MINGW64%\cp" -v client/windows/handler/Debug/lib/exception_handler.lib !BUILD_PREFIX!/lib/breakpad/debug

REM Copy PDB files of debug builds.
"%MINGW64%\cp" -v client/windows/Debug/obj/common/common.pdb !BUILD_PREFIX!/lib/breakpad/debug
"%MINGW64%\cp" -v client/windows/crash_generation/Debug/obj/crash_generation_client/crash_generation_client.pdb !BUILD_PREFIX!/lib/breakpad/debug
"%MINGW64%\cp" -v client/windows/crash_generation/Debug/obj/crash_generation_server/crash_generation_server.pdb !BUILD_PREFIX!/lib/breakpad/debug
"%MINGW64%\cp" -v client/windows/handler/Debug/obj/exception_handler/exception_handler.pdb !BUILD_PREFIX!/lib/breakpad/debug

REM Copy static libraries of release builds.
"%MINGW64%\cp" -v client/windows/Release/lib/common.lib !BUILD_PREFIX!/lib/breakpad/release
"%MINGW64%\cp" -v client/windows/crash_generation/Release/lib/crash_generation_client.lib !BUILD_PREFIX!/lib/breakpad/release
"%MINGW64%\cp" -v client/windows/crash_generation/Release/lib/crash_generation_server.lib !BUILD_PREFIX!/lib/breakpad/release
"%MINGW64%\cp" -v client/windows/handler/Release/lib/exception_handler.lib !BUILD_PREFIX!/lib/breakpad/release

REM Copy PDB files of release builds.
"%MINGW64%\cp" -v client/windows/Release/obj/common/common.pdb !BUILD_PREFIX!/lib/breakpad/release
"%MINGW64%\cp" -v client/windows/crash_generation/Release/obj/crash_generation_client/crash_generation_client.pdb !BUILD_PREFIX!/lib/breakpad/release
"%MINGW64%\cp" -v client/windows/crash_generation/Release/obj/crash_generation_server/crash_generation_server.pdb !BUILD_PREFIX!/lib/breakpad/release
"%MINGW64%\cp" -v client/windows/handler/Release/obj/exception_handler/exception_handler.pdb !BUILD_PREFIX!/lib/breakpad/release

REM Copy header files.
"%MINGW64%\cp" -v client/windows/common/*.h !BUILD_PREFIX!/include/breakpad/client/windows/common
"%MINGW64%\cp" -v client/windows/crash_generation/*.h !BUILD_PREFIX!/include/breakpad/client/windows/crash_generation
"%MINGW64%\cp" -v client/windows/handler/*.h !BUILD_PREFIX!/include/breakpad/client/windows/handler
"%MINGW64%\cp" -v client/windows/sender/*.h !BUILD_PREFIX!/include/breakpad/client/windows/sender
"%MINGW64%\cp" -v common/*.h !BUILD_PREFIX!/include/breakpad/common
"%MINGW64%\cp" -v common/windows/*.h !BUILD_PREFIX!/include/breakpad/common/windows
"%MINGW64%\cp" -v google_breakpad/common/*.h !BUILD_PREFIX!/include/breakpad/google_breakpad/common
"%MINGW64%\cp" -v google_breakpad/processor/*.h !BUILD_PREFIX!/include/breakpad/google_breakpad/processor

:end
popd
if EXIST ".\!SOURCE_PACKAGE!" "%MINGW64%\rm" -rf .\!SOURCE_PACKAGE!
if EXIST ".\gyp" "%MINGW64%\rm" -rf .\gyp
