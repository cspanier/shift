#!/usr/bin/env python3

import sys
import os
from build import Builder

package_name = Builder.package_name_from_filename(__file__)
dependencies = ()


def prepare(builder):
    archive_name = package_name + '-src.tgz'
    builder.download_file(archive_name)
    builder.extract(archive_name)
    return True


def build(builder):
    os.chdir('icu')

    if builder.toolset.startswith('msvc'):
        """
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
        """
    else:
        os.chdir('source')
        # cross builds need:
        #   --host=armv5l-timesys-linux-gnueabi
        #   --with-cross-build=/path/to/native/built-icu-source-path
        builder.configure()
        builder.make(install=True)


def cleanup(builder):
    builder.remove_folder('icu')


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
