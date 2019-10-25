#!/usr/bin/env python3

import sys
import os
from pathlib import Path
from build import Builder

package_name = Builder.package_name_from_filename(__file__)
dependencies = ()


def prepare(builder):
    # There is a special Windows package available with precompiled assembler files and header files in Windows format.
    if builder.host_system == "windows":
        archive_name = 'jpegsr9c.zip'
    else:
        archive_name = 'jpegsrc.v9c.tar.gz'
    builder.extract(archive_name)
    return True


def build(builder):
    os.chdir(package_name)

    if builder.toolset.startswith('msvc'):
        builder.dos2unix(['**/makefile*'])
        builder.apply_patch(['-p1'], '../jpeg-9c-makefile.patch')
        builder.nmake(makefile='makefile.vc', args=['setup-v15'])
        builder.dos2unix(['**/*.vcxproj', '**/*.sln'])
        builder.apply_patch(['-p1'], '../jpeg-9c-{}.patch'.format(builder.toolset))
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
        builder.make(install=True)


def cleanup(builder):
    builder.remove_folder(package_name)


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
