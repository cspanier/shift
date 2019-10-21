#!/usr/bin/env python3

import sys
import os
import multiprocessing
from pathlib import Path
from build import Builder
import subprocess

package_name = Builder.package_name_from_filename(__file__)
# ToDo: Add ICU4C to this list
dependencies = (
    'zlib-1.2.11',
    'bzip2-1.0.8')


def prepare(builder):
    archive_name = package_name + '.tar.bz2'
    builder.download_file(archive_name)
    builder.extract(archive_name)
    return True


def build(builder):
    os.chdir(package_name)

    environment = builder.setup_env()

    if builder.host_system == 'windows':
        subprocess.check_call(['bootstrap.bat'], env=environment)
    else:
        bootstrap_args = ['./bootstrap.sh']
        if builder.toolset == 'clang':
            bootstrap_args.append('--with-toolset=clang')
        subprocess.check_call(bootstrap_args, env=environment)

    b2_args = ['./b2',
               '--layout=versioned',
               '--build-type=complete',
               '--without-mpi',
               '--without-graph_parallel',
               '--without-python']
    if builder.host_system == 'linux':
        b2_args.append('valgrind=on')
    if builder.toolset == 'gcc':
        b2_args.append('toolset=gcc')
    elif builder.toolset == 'clang':
        b2_args.append('toolset=clang')
    elif builder.toolset == 'msvc15':
        b2_args.append('toolset=msvc-14.1')
    elif builder.toolset == 'msvc16':
        b2_args.append('toolset=msvc-14.2')
    if builder.toolset == 'clang' and builder.stdlib == 'libc++':
        b2_args.extend(['cxxflags="-stdlib=libc++ -std=c++14"',
                        'linkflags="-stdlib=libc++"'])
    elif builder.toolset in ('gcc', 'clang'):
        b2_args.append('cxxflags="-std=c++14"')
    b2_args.append('architecture={}'.format('x86' if builder.target_platform.startswith('x86_') else 'unknown'))
    b2_args.append('address-model={}'.format(builder.target_platform_bits))
    b2_args.extend(['threading=multi',
                    'link=static,shared'])
    if builder.target_system == 'windows':
        b2_args.append('define=BOOST_USE_WINAPI_VERSION=0x0A00')
    b2_args.extend([
        '-sZLIB_BINARY=zlib',
        '-sZLIB_INCLUDE={}/include'.format(builder.install_prefix),
        '-sZLIB_LIBPATH={}/lib'.format(builder.install_prefix),
        '-sBZIP2_BINARY=libbz2',
        '-sBZIP2_INCLUDE={}/include'.format(builder.install_prefix),
        '-sBZIP2_LIBPATH={}/lib'.format(builder.install_prefix),
        '-sICU_PATH={}'.format(builder.install_prefix),
        '--prefix={}'.format(builder.install_prefix),
        'install',
        '-j{}'.format(multiprocessing.cpu_count())])
    subprocess.check_call(b2_args, env=environment)


def cleanup(builder):
    builder.remove_folder(package_name)


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
