@echo off
setlocal EnableDelayedExpansion

if "%1"=="" (
  echo Usage: %0 (build-folder)
  goto :end
)

set PWD=!CD!
pushd !CD!
cd ..\shift
for /R %%i in (*.cpp) do (
  echo "Processing %%i..."
  clang-tidy.exe ^
    -p "../%1/" ^
    -header-filter=shift/ ^
    -config="{Checks: '-*,modernize-deprecated-headers,modernize-raw-string-literal,modernize-use-auto,modernize-use-equals-default,modernize-use-override'}" ^
    -fix ^
    -format-style=file ^
    "%%i"
)
popd

:end
