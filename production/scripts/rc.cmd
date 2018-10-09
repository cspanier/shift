@echo off

pushd %CD%
cd %~dp0..

for /f "delims=" %%i in ('where /F magick') do (
  if NOT ERRORLEVEL 0 goto error
  set IMAGE_MAGICK=%%i
  goto found_image_magick
)
if NOT ERRORLEVEL 0 goto error
:found_image_magick

REM Call resource compiler
bin\shift.tools.rc.2_0.x86_64.vc141 ^
  --log-level debug ^
  --verbose 2 ^
  --image-magick %IMAGE_MAGICK%
if NOT ERRORLEVEL 0 goto error
goto end

:error
echo ### Last command exited with error code %ERRORLEVEL%
goto end

:end
popd
pause
