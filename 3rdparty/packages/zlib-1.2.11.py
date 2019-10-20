#!/usr/bin/env python3

import sys
import os
from pathlib import Path

archive_name = ''


def prepare(builder, package_name):
    global archive_name
    archive_name = package_name + '.tar.gz'
    builder.download_file(archive_name)
    builder.extract(archive_name)
    return True


def build(builder, package_name):
    os.chdir(package_name)

    package_dir = os.getcwd()
    os.mkdir('build', 0o750)
    os.chdir('build')
    builder.cmake(build_type='Debug')
    builder.cmake_install()

    os.chdir(package_dir)
    builder.remove_folder('build')

    os.mkdir('build', 0o750)
    os.chdir('build')
    builder.cmake(build_type='Release')
    builder.cmake_install()


def cleanup(builder, package_name):
    builder.remove_folder(package_name)


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
