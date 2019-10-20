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
    # ToDo: builder.check_file_exists zlib, lzma, jpeg and zstd libraries.
    return True


def build(builder, package_name):
    os.chdir(package_name)

    if builder.toolset.startswith('msvc'):
        builder.apply_patch(['-p1'], '../{}-nmake.patch'.format(archive_name))
        builder.nmake(makefile='makefile.vc')

        # Manually install files
        builder.install(source=Path('libtiff'),
                        patterns=['*.dll'],
                        destination=Path('bin'))
        builder.install(source=Path('libtiff'),
                        patterns=['*.lib', '*.pdb'],
                        destination=Path('lib'))
        builder.install(source=Path('libtiff'),
                        patterns=['*.h'],
                        destination=Path('include'))
    else:
        builder.configure(args=[
            '--with-zlib-include-dir={}/include'.format(builder.install_prefix),
            '--with-zlib-lib-dir={}/lib'.format(builder.install_prefix),
            '--with-lzma-include-dir={}/include'.format(builder.install_prefix),
            '--with-lzma-lib-dir={}/lib'.format(builder.install_prefix),
            '--with-jpeg-include-dir={}/include'.format(builder.install_prefix),
            '--with-jpeg-lib-dir={}/lib'.format(builder.install_prefix),
            '--with-zstd-include-dir={}/include'.format(builder.install_prefix),
            '--with-zstd-lib-dir={}/lib'.format(builder.install_prefix)
        ])
        builder.make(install=True)


def cleanup(builder, package_name):
    builder.remove_folder(package_name)


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
