#!/usr/bin/env python3

import sys
import os
from pathlib import Path
from build import Builder

archive_name = ''


def prepare(builder, package_name):
    global archive_name
    archive_name = package_name + '.tar.gz'
    builder.download_file(archive_name)
    builder.extract(archive_name)
    return True


def build(builder, package_name):
    os.chdir(package_name)
    if builder.toolset.startswith('msvc'):
        builder.nmake(makefile='makefile.msc')

        # Manually install files
        builder.install(source=Path('.'),
                        patterns=['*.dll'],
                        destination=Path('bin'))
        builder.install(source=Path('.'),
                        patterns=['*.a', '.so', '*.lib', '*.pdb'],
                        destination=Path('lib'))
        builder.install(source=Path('.'),
                        patterns=['bzlib.h'],
                        destination=Path('include'))

        # CMake's find BZIP2 library script expects libbz2.lib to be named bz2.lib
        os.link((builder.install_prefix / 'lib' / 'libbz2.lib').as_posix(),
                (builder.install_prefix / 'lib' / 'bz2.lib').as_posix())
    elif builder.toolset in ('gcc', 'clang'):
        builder.make(env_vars={'CFLAGS': '-Wall -Winline -O2 -g -D_FILE_OFFSET_BITS=64 -fPIC'})


def cleanup(builder, package_name):
    builder.remove_folder(package_name)


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
