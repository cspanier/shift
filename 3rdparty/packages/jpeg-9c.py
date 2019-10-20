#!/usr/bin/env python3

import sys
import os
from pathlib import Path
from build import Builder

import fileinput
import re

archive_name = ''


def prepare(builder, package_name):
    global archive_name
    if builder.host_system == "windows":
        archive_name = 'jpegsr9c.zip'
    else:
        archive_name = 'jpegsrc.v9c.tar.gz'
    builder.download_file(archive_name)
    builder.extract(archive_name)
    return True


def build(builder, package_name):
    os.chdir(package_name)

    if builder.toolset.startswith('msvc'):
        builder.apply_patch(['-p1'], '../jpeg-9c-makefile.patch')
        builder.nmake(makefile='makefile.vc', args=['setup-v15'])
        builder.apply_patch(['-p1'], '../jpeg-9c-{}}.patch'.format(builder.toolset))
        builder.msbuild('jpeg.sln', 'Release')

        build_folder = '.'
        if builder.target_platform_bits == 64:
            build_folder = 'x64'

        # Manually install files
        builder.install(source=Path('{}/Release'.format(build_folder)),
                        patterns=['*.lib', '*.pdb'],
                        destination=Path('lib'))
        builder.install(source=Path('.'),
                        patterns=['*.h'],
                        destination=Path('include'))
    else:
        builder.configure(args=['--enable-shared', '--enable-static'])
        builder.make()


def cleanup(builder, package_name):
    #builder.remove_folder(package_name)
    x = 1


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
