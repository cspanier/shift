#!/usr/bin/env python3

import sys
import os
from pathlib import Path
from build import Builder

package_name = Builder.package_name_from_filename(__file__)
dependencies = (
    'zlib-1.2.11',
    'bzip2-1.0.8',
    'jpeg-9c',
    'zstd-be3bd70')


def prepare(builder):
    archive_name = package_name + '.tar.gz'
    builder.download_file(archive_name)
    builder.extract(archive_name)
    return True


def build(builder):
    os.chdir(package_name)

    if builder.toolset.startswith('msvc'):
        builder.apply_patch(['-p1'], '../{}-nmake.patch'.format(package_name))
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


def cleanup(builder):
    builder.remove_folder(package_name)


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
