#!/usr/bin/env python3

import sys
import os
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


def cleanup(builder):
    builder.remove_folder(package_name)


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
