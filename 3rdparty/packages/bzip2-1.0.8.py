#!/usr/bin/env python3

import sys
import os
from pathlib import Path
from build import Builder

package_name = Builder.package_name_from_filename(__file__)
dependencies = ()


def prepare(builder):
    archive_name = package_name + '.tar.gz'
    builder.extract(archive_name)
    return True


def build(builder):
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
        source = builder.install_prefix / 'lib' / 'libbz2.lib'
        target = builder.install_prefix / 'lib' / 'bz2.lib'
        if target.exists():
            os.remove(target.as_posix())
        os.link(source.as_posix(), target.as_posix())
    elif builder.toolset in ('gcc', 'clang'):
        builder.make(env_vars={'CFLAGS': '-Wall -Winline -O2 -g -D_FILE_OFFSET_BITS=64 -fPIC'},
                     install=True)


def cleanup(builder):
    builder.remove_folder(package_name)


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
