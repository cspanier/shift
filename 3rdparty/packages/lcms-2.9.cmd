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
set SOURCE_PACKAGE=lcms-2.9
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

if /i "%1"=="msvc15" (
  echo Patching project files for %1
  "%MINGW64%\patch" -p1 < ..\!SOURCE_PACKAGE!-msvc15.patch
  IF NOT ERRORLEVEL 0 goto end
)

cd Projects\VC2017\lcms2_static

REM msbuild lcms2_static.vcxproj /t:Rebuild /p:Configuration=Debug;Platform=%BUILD_PLATFORM%
msbuild lcms2_static.vcxproj /t:Rebuild /p:Configuration=Release;Platform=%BUILD_PLATFORM%

cd ..\..\..

"%MINGW64%\cp" -v Lib/MS/lcms2_static.lib %BUILD_PREFIX%/lib
"%MINGW64%\cp" -v include/*.h %BUILD_PREFIX%/include

:end
popd
if EXIST ".\!SOURCE_PACKAGE!" "%MINGW64%\rm" -rf .\!SOURCE_PACKAGE!
