#!/usr/bin/env python3

import sys
import os
import shutil
from build import Builder

package_name = Builder.package_name_from_filename(__file__)
dependencies = ()


def prepare(builder):
    archive_name = package_name + '.tar.gz'
    builder.extract(archive_name)
    return True


def build(builder):
    os.chdir(package_name)
    os.mkdir('build', 0o750)
    os.chdir('build')
    builder.cmake(build_type='Release', install=True)

    if builder.host_system == 'windows':
        # The install target places libs in a Release sub-folder on Windows.
        for filename in (builder.install_prefix / 'lib' / 'Release').glob('*'):
            shutil.move(filename, builder.install_prefix / 'lib' / filename.name)
        shutil.rmtree(builder.install_prefix / 'lib' / 'Release')

        # Remove license file which gets installed directly to PREFIX path on Windows.
        os.remove(builder.install_prefix / 'LICENSE')


def cleanup(builder):
    builder.remove_folder(package_name)


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
