#!/usr/bin/env python3


import sys
import errno
import os
import platform
import multiprocessing
import io
from contextlib import contextmanager
import glob
from pathlib import Path
import subprocess
import argparse
import importlib
import importlib.util
import tarfile
import zipfile
import shutil
import urllib.request


class Builder:
    def __init__(self):
        # Find available packages.
        available_packages = []
        for filename in glob.glob("*.py"):
            if filename != 'build.py':
                package_name, file_extension = os.path.splitext(filename)
                available_packages.append(package_name)

        self.host_system = platform.system().lower()

        # https://docs.python.org/3.3/library/argparse.html
        parser = argparse.ArgumentParser(
            description='Build 3rd party packages in a uniform way.')
        parser.add_argument('-t', '--toolset', required=True,
                            choices=['gcc', 'clang', 'msvc15', 'msvc16'],
                            help='The compiler toolset to use.')
        parser.add_argument('-p', '--target-platform',
                            choices=['x86_64', 'x86_32', 'arm64', 'arm'],
                            default='x86_64',
                            help='The target platform to build for.')
        parser.add_argument('-s', '--target-system',
                            choices=['linux', 'windows', 'darwin'],
                            default=self.host_system,
                            help='The target operating system to build for.')
        parser.add_argument('-l', '--stdlib', choices=['libstdc++', 'libc++'],
                            default='libstdc++',
                            help='When using Clang this argument selects ' +
                                 'which standard library to use.')
        parser.add_argument('packages', nargs='+',
                            choices=available_packages,
                            help='A list of packages to build.')

        args = parser.parse_args()
        self.toolset = args.toolset
        self.target_platform = args.target_platform
        if self.target_platform in ('x86_64', 'arm64'):
            self.target_platform_bits = 64
        elif self.target_platform in ('x86_32', 'arm'):
            self.target_platform_bits = 32
        else:
            raise NotImplementedError(
                'Unknown pointer size of target platform {}.'.format(
                    self.target_platform))
        self.target_system = args.target_system
        self.stdlib = args.stdlib
        self.packages = args.packages
        install_prefix_folder = Path('{}-{}-{}'.format(
            self.target_platform,
            self.target_system,
            self.toolset))

        if self.toolset == 'gcc':
            self.cmake_generator = 'Ninja'
        elif self.toolset == 'clang':
            self.cmake_generator = 'Ninja'
            # Clang may build against different standard libraries
            install_prefix_folder = Path('{}-{}-{}-{}'.format(
                self.target_platform,
                self.target_system,
                self.toolset,
                self.stdlib))
        elif self.toolset == 'msvc16':
            self.cmake_generator = 'Visual Studio 16 2019'
            if self.target_platform == 'x86_64':
                self.cmake_generator_platform = 'x64'
            elif self.target_platform == 'x86_32':
                self.cmake_generator_platform = 'Win32'
            elif self.target_platform == 'arm':
                self.cmake_generator_platform = 'ARM'
            elif self.target_platform == 'arm64':
                self.cmake_generator_platform = 'ARM64'
            else:
                raise NotImplementedError(
                    'The selected toolset does not support building for the ' +
                    'selected target platform.',
                    self.target_platform, self.toolset)
        elif self.toolset == 'msvc15':
            if self.target_platform == 'x86_64':
                self.cmake_generator = 'Visual Studio 15 2017 Win64'
            elif self.target_platform == 'x86_32':
                self.cmake_generator = 'Visual Studio 15 2017'

        self.install_prefix = Path.cwd() / '..' / install_prefix_folder
        os.makedirs(self.install_prefix, mode=0o755, exist_ok=True)

    def run(self):
        for package_name in self.packages:
            print('Building package ' + package_name)
            # Dynamically import source file package.
            spec = importlib.util.spec_from_file_location(package_name,
                                                          package_name + '.py')
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            pwd = os.getcwd()
            try:
                if module.prepare(self, package_name):
                    module.build(self, package_name)
            finally:
                os.chdir(pwd)
                module.cleanup(self, package_name)

    #
    @staticmethod
    def download_file(filename):
        if not os.path.exists(filename):
            try:
                print('* Downloading archive "{}" ...'.format(filename), end=' ')
                urllib.request.urlretrieve(
                    'https://boxie.eu/3rdparty/{}'.format(filename),
                    filename,
                    Builder._show_download_progress)
            except Exception:
                print('FAILED')
                raise

    #
    @staticmethod
    def extract(filename):
        Builder.check_file_exists(filename)
        try:
            print('* Extracting archive "{}" ...'.format(filename), end=' ')
            file_name, file_extension = os.path.splitext(filename)
            if file_extension.lower() == '.zip':
                with zipfile.ZipFile(filename, 'r') as zip_ref:
                    zip_ref.extractall('.')
            else:
                archive = tarfile.open(filename)
                archive.extractall()
            print('OK')
        except Exception:
            print('FAILED')
            raise

    #
    @staticmethod
    def apply_patch(args, filename):
        patch_args = ['patch']
        patch_args.extend(args)
        subprocess.run(patch_args, input=Path(filename).read_text())

    #
    def cmake(self, source_path="../", build_type=None, args=[], env_vars={}):
        environment = self.setup_env(env_vars)
        cmake_args = ['cmake', source_path,
                      '-G', self.cmake_generator,
                      '-DCMAKE_INSTALL_PREFIX:PATH={}'.format(
                          self.install_prefix.as_posix())]
        if self.cmake_generator_platform:
            cmake_args.extend(['-A', self.cmake_generator_platform])
        if build_type:
            cmake_args.append('-DCMAKE_BUILD_TYPE=' + build_type)
        cmake_args.extend(args)
        subprocess.check_call(cmake_args, env=environment)

    #
    def cmake_install(self, env_vars={}):
        environment = self.setup_env(env_vars)
        cmake_args = ['cmake', '--build', '.', '--target', 'install']
        subprocess.check_call(cmake_args, env=environment)

    #
    def configure(self, env_vars={}, args=[]):
        environment = self.setup_env(env_vars)
        configure_args = ['/bin/env', './configure',
                          '--prefix={}'.format(self.install_prefix)]
        configure_args.extend(args)
        subprocess.check_call(configure_args, env=environment)

    #
    def make(self, env_vars={}, install=False):
        environment = self.setup_env(env_vars)
        make_args = ['make',
                     'CC={}'.format(environment['CC']),
                     'CXX={}'.format(environment['CXX'])]
        if 'CFLAGS' in environment:
            make_args.append("CFLAGS={}".format(environment['CFLAGS']))
        if 'CXXFLAGS' in environment:
            make_args.append("CXXFLAGS={}".format(environment['CXXFLAGS']))
        make_args.append('-j{}'.format(multiprocessing.cpu_count()))
        if install:
            make_args.extend(['install',
                              "PREFIX='{}'".format(self.install_prefix)])
        subprocess.check_call(make_args, env=environment)

    #
    def ninja(self, env_vars={}):
        environment = self.setup_env(env_vars)
        make_args = ['ninja']
        subprocess.check_call(make_args, env=environment)

    #
    def msbuild(self, msvs_proj, build_type, env_vars={}):
        if not self.toolset.startswith('msvc'):
            raise NotImplementedError('msbuild only supports msvc toolsets.')

        environment = self.setup_env(env_vars)
        make_args = ['msbuild', msvs_proj, '/t:Rebuild',
                     '/p:Configuration={};Platform={}'.format(
                         build_type, self.cmake_generator_platform)]
        subprocess.check_call(make_args, env=environment)

    #
    def nmake(self, makefile, args=[], env_vars={}):
        environment = self.setup_env(env_vars)
        make_args = ['nmake', '-f', makefile]
        make_args.extend(args)
        subprocess.check_call(make_args, env=environment)

    #
    def install(self, source, patterns=[], destination=Path('.')):
        source_folder = Path.cwd() / Path(source)
        for pattern in patterns:
            for source_path in source_folder.glob(pattern):
                source_file = source_path.relative_to(source_folder)
                source_path = source_path.resolve()
                destination_folder = (self.install_prefix /
                                      destination /
                                      source_file.parent).resolve()
                os.makedirs(destination_folder, mode=0o755, exist_ok=True)
                destination_path = destination_folder / source_file.name
                print('* install {} -> {}'.format(source_path.as_posix(),
                                                  destination_path.as_posix()))
                shutil.copyfile(source_path.as_posix(),
                                destination_path.as_posix())

    #
    @staticmethod
    def check_file_exists(filename):
        if not os.path.exists(filename):
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT),
                                    filename)

    #
    @staticmethod
    def remove_folder(path):
        try:
            print('* Removing folder "' + path + '" ...', end=' ')
            if os.path.exists(path) and os.path.isdir(path):
                shutil.rmtree(path)
            print('OK')
        except Exception:
            print('FAILED')
            raise

    #
    def setup_env(self, env_vars={}):
        environment = os.environ.copy()

        env_var_separator = ';' if self.host_system == 'windows' else ':'

        include = ''
        if 'INCLUDE' in environment:
            include = environment['INCLUDE']
        environment['INCLUDE'] = include + env_var_separator +\
                                 (self.install_prefix / 'include').as_posix()

        lib = ''
        if 'LIB' in environment:
            lib = environment['LIB']
        environment['LIB'] = lib + env_var_separator +\
                             (self.install_prefix / 'lib').as_posix()

        for key, value in env_vars.items():
            if key[0] == '+':
                key = key[1:]
                environment[key] = environment[key] + env_var_separator + value
            else:
                environment[key] = value
        if self.toolset == 'gcc':
            environment['CC'] = 'gcc'
            environment['CXX'] = 'g++'
        elif self.toolset == 'clang':
            environment['CC'] = 'clang'
            environment['CXX'] = 'clang++'
            cxx_flags = ''
            if 'CXXFLAGS' in environment:
                cxx_flags = environment['CXXFLAGS']
            cxx_flags = '{} -stdlib={}'.format(cxx_flags, self.stdlib)
        return environment

    #
    @staticmethod
    def _show_download_progress(block_num, block_size, total_size):
        downloaded = block_num * block_size
        if block_num != 0:
            print('\b' * 40, end='')
        if downloaded < total_size:
            progress = int(downloaded * 40 / total_size)
            print('#' * progress, end='')
            print('.' * (40 - progress), end='', flush=True)
        else:
            print('OK' + ' ' * 38, flush=True)

    toolset = ''
    host_system = ''
    target_platform = ''
    target_platform_bits = 64
    target_system = ''
    stdlib = 'libstdc++'
    packages = []
    cmake_generator = 'Ninja'
    cmake_generator_platform = ''
    install_prefix = Path('.')


if __name__ == "__main__":
    builder = Builder()
    builder.run()
