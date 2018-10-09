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
set SOURCE_PACKAGE=icu4c-60_2-src
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  "%MINGW64%\..\..\mingw64\bin\curl.exe -L -J -O "https://boxie.eu/3rdparty/!SOURCE_PACKAGE!.zip"
)
if NOT EXIST ".\!SOURCE_PACKAGE!.zip" (
  echo Error: Source package !SOURCE_PACKAGE!.zip is missing.
  goto end
)

"%MINGW64%\unzip" .\!SOURCE_PACKAGE!.zip
if %ERRORLEVEL% NEQ 0 goto end
pushd %CD%
cd icu

if /i "%1"=="msvc15" (
  echo Patching project files for %1
  "%MINGW64%\patch" -p1 < ..\icu4c-60_2-msvc15.patch
  IF NOT ERRORLEVEL 0 pause
) else (
  echo Unknown compiler.
  goto end
)

msbuild source\allinone\allinone.sln /t:Rebuild /p:Configuration=Debug;Platform=%BUILD_PLATFORM%
msbuild source\allinone\allinone.sln /t:Rebuild /p:Configuration=Release;Platform=%BUILD_PLATFORM%

if /i "%BUILD_BITS%"=="32" (
  set BUILD_ICU4C_BINFOLDER=bin
  set BUILD_ICU4C_LIBFOLDER=lib
) else if /i "%BUILD_BITS%"=="64" (
  set BUILD_ICU4C_BINFOLDER=bin64
  set BUILD_ICU4C_LIBFOLDER=lib64
)
"%MINGW64%\cp" -v %BUILD_ICU4C_BINFOLDER%\*.exe !BUILD_PREFIX!\bin
"%MINGW64%\cp" -v %BUILD_ICU4C_BINFOLDER%\*.dll !BUILD_PREFIX!\bin
"%MINGW64%\cp" -v %BUILD_ICU4C_LIBFOLDER%\*.lib !BUILD_PREFIX!\lib
"%MINGW64%\cp" -v %BUILD_ICU4C_LIBFOLDER%\*.pdb !BUILD_PREFIX!\lib
"%MINGW64%\cp" -v -r include/* !BUILD_PREFIX!\include

:end
popd
if EXIST ".\icu" "%MINGW64%\rm" -rf .\icu
