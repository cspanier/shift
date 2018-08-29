@echo off

setlocal EnableDelayedExpansion
pushd !CD!

set WINSDK_VERSION=10.0.16299.0
set HOST_COMPILER=vc141
set TRIPLE=x86_64-windows-%HOST_COMPILER%
set QT5_PATH=C:/Qt/Qt5.11.0/5.11.0/msvc2017_64
set CMAKE_GENERATOR=Visual Studio 15 2017 Win64

if "%WINSDK_VERSION%"=="" (
  echo "WINSDK_VERSION environment variable not set."
  goto end
)
if "%TRIPLE%"=="" (
  echo "TRIPLE environment variable not set."
  goto end
)
if "%QT5_PATH%"=="" (
  echo "QT5_PATH environment variable not set."
  goto end
)
if "%CMAKE_GENERATOR%"=="" (
  echo "CMAKE_GENERATOR environment variable not set."
  goto end
)

pushd !CD!
if "!CMAKE_PREFIX_PATH!"=="" (
  for /L %%i in (1, 1, 3) do (
    cd ..
    if EXIST 3rdparty/%TRIPLE% (
      set CMAKE_PREFIX_PATH=!CD!/3rdparty/%TRIPLE%
      goto found_folder
    )
  )
  echo "Cannot find cmake prefix path."
  goto end
)
:found_folder
popd

REM Replace backslashes with forward slashes in variable CMAKE_PREFIX_PATH
set CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH:\=/!

REM Remove quotation marks from several variables
set CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH:"=!
set CMAKE_GENERATOR=!CMAKE_GENERATOR:"=!

echo CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH!
echo CMAKE_GENERATOR=!CMAKE_GENERATOR!

cd ..
if not exist build-!TRIPLE! mkdir build-!TRIPLE!
cd build-!TRIPLE!
if exist CMakeCache.txt del CMakeCache.txt
call cmake.exe -G "!CMAKE_GENERATOR!" .. ^
  -DCMAKE_PREFIX_PATH:PATH="!CMAKE_PREFIX_PATH!;!QT5_PATH!" ^
  -DCMAKE_INSTALL_PREFIX:PATH="%~dp0/../production" ^
  -DCMAKE_TOOLCHAIN_FILE:FILEPATH="%~dp0/../cmake/Toolchain-%TRIPLE%.cmake" ^
  -DSHIFT_DEBUG:BOOL=OFF ^
  %*

goto end

:end
popd

pause
