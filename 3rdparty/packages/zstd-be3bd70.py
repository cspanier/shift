#!/usr/bin/env python3

import sys
import os
from pathlib import Path
from build import Builder

package_name = Builder.package_name_from_filename(__file__)
dependencies = ()


def prepare(builder):
    archive_name = package_name + '.zip'
    builder.download_file(archive_name)
    builder.extract(archive_name)
    return True


def build(builder):
    os.chdir((Path(package_name) / 'build' / 'cmake').as_posix())

    os.mkdir('build', 0o750)
    os.chdir('build')
    builder.cmake(build_type='Release', args=[
        '-DZSTD_BUILD_CONTRIB=OFF',
        '-DZSTD_BUILD_TESTS=OFF',
        '-DZSTD_BUILD_SHARED=OFF',
        '-DZSTD_BUILD_STATIC=ON'],
                  install=True)
    # Install additional files required to access advanced APIs.
    builder.install(source=Path('../../../lib'),
                    patterns=['deprecated/zbuff.h',
                              'dictBuilder/zdict.h',
                              'dictBuilder/cover.h',
                              'common/zstd_errors.h'],
                    destination=Path('include'))


def cleanup(builder):
    builder.remove_folder(package_name)


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
