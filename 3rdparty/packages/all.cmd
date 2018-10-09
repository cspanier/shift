@echo off

echo ========== gsl ==========
call gsl %1 %2
echo ========== zlib1211 ==========
call zlib1211 %1 %2
echo ========== bzip2-1.0.6 ==========
call bzip2-1.0.6 %1 %2
if /i "%2"=="x86_32" (
  REM Skip the following libraries.
) else (
  echo ========== jpeg-9c ==========
  call jpeg-9c %1 %2
  echo ========== tiff-4.0.9 ==========
  call tiff-4.0.9 %1 %2
  echo ========== libpng-1.6.34 ==========
  call libpng-1.6.34 %1 %2
)
echo ========== icu4c-60_2 ==========
call icu4c-60_2 %1 %2
echo ========== boost_1_67_0 ==========
call boost_1_68_0 %1 %2
echo ========== breakpad-1459e5d ==========
call breakpad-1459e5d %1 %2
echo ========== glfw-3.2.1 ==========
call glfw-3.2.1 %1 %2
