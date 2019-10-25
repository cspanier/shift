#!/usr/bin/env python3

import sys
import os
import re
import subprocess
import shutil
from pathlib import Path
from build import Builder

package_name = Builder.package_name_from_filename(__file__)
dependencies = ()


def prepare(builder):
    archive_name = package_name + '-src.tgz'
    builder.extract(archive_name)
    return True


def build(builder):
    os.chdir('icu')
    os.chdir('source')

    """
    List of relevant configure arguments:
        --enable-shared         build shared libraries default=yes
        --enable-static         build static libraries default=no
        --enable-extras         build ICU extras default=yes
        --enable-layoutex         build ICU's Paragraph Layout library default=yes.
                  icu-le-hb must be installed via pkg-config. See http://harfbuzz.org

        --enable-tests          build ICU tests default=yes
        --enable-samples        build ICU samples default=yes

        --with-library-bits=bits specify how many bits to use for the library (32, 64, 64else32, nochange) default=nochange
        --with-data-packaging     specify how to package ICU data. Possible values:
              files    raw files (.res, etc)
              archive  build a single icudtXX.dat file
              library  shared library (.dll/.so/etc.)
              static   static library (.a/.lib/etc.)
              auto     build shared if possible (default)
                 See http://userguide.icu-project.org/icudata for more info.
    """
    common_configure_args = ['--disable-shared', '--enable-static', '--disable-extras', '--disable-layoutex',
                             '--disable-tests', '--disable-samples',
                             '--with-library-bits={}'.format(builder.target_platform_bits),
                             '--with-data-packaging=static']
    if builder.toolset.startswith('msvc'):
        environment = builder.setup_env()
        # We need to add the /utf-8 flag to make cl treat source files UTF8 encoded.
        environment['CFLAGS'] = '{} /utf-8'.format(environment.get('CFLAGS', ''))
        environment['CXXFLAGS'] = '{} /utf-8'.format(environment.get('CXXFLAGS', ''))

        # Convert Windows path to Cygwin path.
        install_prefix = re.sub(r'(.):/(.*)', r'/cygdrive/\1/\2', builder.install_prefix.as_posix())
        subprocess.check_call([(builder.cygwin / 'bin' / 'bash').as_posix(), 'runConfigureICU', 'Cygwin/MSVC',
                               common_configure_args,
                               '--prefix={}'.format(install_prefix)],
                              env=environment)

        # Replace linker parameter '-o' to '/out' in all Makefiles.
        for filename in Path('.').glob('**/Makefile'):
            print(filename)
            with open(filename.as_posix(), 'r') as file:
                file_data = file.read()
            file_data = re.sub(r'(\$\(LINK\.cc\).*) -o (.*)', r'\1 /OUT:\2', file_data)
            with open(filename.as_posix(), 'w') as file:
                file.write(file_data)

        builder.make()
        builder.make(install=True)

        # The install target places DLL files in the lib folder, even though they actually belong in the bin folder.
        for filename in (builder.install_prefix / 'lib').glob('**/icu*64.dll'):
            shutil.move(filename, builder.install_prefix / 'bin' / filename.name)
    else:
        builder.configure(args=common_configure_args)
        builder.make(install=True)


def cleanup(builder):
    builder.remove_folder('icu')


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
