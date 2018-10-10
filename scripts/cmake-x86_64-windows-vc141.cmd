@echo off

set WINSDK_VERSION=10.0.17134.0
set HOST_COMPILER=vc141
set TRIPLE=x86_64-windows-%HOST_COMPILER%
set QT5_VERSION=5.11.2
if EXIST C:/Qt/%QT5_VERSION%/msvc2017_64 (
  REM Default path for online installer.
  set QT5_PATH=C:/Qt/%QT5_VERSION%/msvc2017_64
) else if EXIST C:/Qt/Qt%QT5_VERSION%/%QT5_VERSION%/msvc2017_64 (
  REM Default path for offline installer.
  set QT5_PATH=C:/Qt/Qt%QT5_VERSION%/%QT5_VERSION%/msvc2017_64
)
set CMAKE_GENERATOR=Visual Studio 15 2017 Win64

if "%VisualStudioVersion"=="15.0" (
  if "%Platform%"=="X64" (
    goto skip_vsvars_bat
  )
)
REM Syntax: vsdevcmd.bat [options]
REM [options] :
REM     -arch=architecture : Architecture for compiled binaries/libraries
REM            * x86 [default]
REM            * amd64
REM            * arm
REM     -host_arch=architecture : Architecture of compiler binaries
REM            * x86 [default]
REM            * amd64
REM     -winsdk=version : Version of Windows SDK to select.
REM            ** 10.0.xxyyzz.0 : Windows 10 SDK (e.g 10.0.10240.0)
REM                               [default : Latest Windows 10 SDK]
REM            ** 8.1 : Windows 8.1 SDK
REM            ** none : Do not setup Windows SDK variables.
REM                      For use with build systems that prefer to
REM                      determine Windows SDK version independently.
REM     -app_platform=platform : Application Platform Target Type.
REM            ** Desktop : Classic Win32 Apps          [default]
REM            ** UWP : Universal Windows Platform Apps
REM     -no_ext : Only scripts from [VS150COMNTOOLS]\VsDevCmd\Core
REM               directory are run during initialization.
REM     -no_logo : Suppress printing of the developer command prompt banner.
REM     -test : Run smoke tests to verify environment integrity after
REM             after initialization (requires all other arguments
REM             to be the same other than -test).
REM     -help : prints this help message.
REM ToDo: Find Visual Studio 2017 installation path using registry.
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\Common7\Tools\VsDevCmd.bat" ^
  -arch=amd64 ^
  -host_arch=amd64 ^
  -winsdk=%WINSDK_VERSION%
:skip_vsvars_bat

cmake.cmd -Thost=x64 %*
pause
