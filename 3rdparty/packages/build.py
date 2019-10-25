#!/usr/bin/env python3

import errno
import os
import platform
import multiprocessing
import re
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
        self.host_system = platform.system().lower()

        # Find available packages.
        for filename in Path.cwd().glob("*.py"):
            if filename != 'build.py':
                self.available_packages.append(self.package_name_from_filename(filename))

        parser = argparse.ArgumentParser(
            description='Build 3rd party packages in a uniform way.')
        parser.add_argument('-t', '--toolset', required=True,
                            choices=('gcc', 'clang', 'msvc15', 'msvc16'),
                            help='The compiler toolset to use.')
        parser.add_argument('-p', '--target-platform',
                            choices=('x86_64', 'x86_32', 'arm64', 'arm'),
                            default='x86_64',
                            help='The target platform to build for.')
        parser.add_argument('-s', '--target-system',
                            choices=('linux', 'windows', 'darwin'),
                            default=self.host_system,
                            help='The target operating system to build for.')
        parser.add_argument('-l', '--stdlib', choices=['libstdc++', 'libc++'],
                            default='libstdc++',
                            help='When using Clang this argument selects ' +
                                 'which standard library to use.')
        parser.add_argument('-n', '--no-dependencies',
                            action='store_const', const=True, default=False,
                            help="Don't automatically build package dependencies.")
        parser.add_argument('-k', '--keep-temporary-files',
                            action='store_const', const=True, default=False,
                            help="Don't delete temporary build folders after use. This may be useful for debugging.")
        parser.add_argument('-m', '--mirror', default=self.mirror,
                            help='URL of Server where to download missing files from.')
        if self.host_system == 'windows':
            parser.add_argument('--cygwin', default=self._detect_cygwin().as_posix(),
                                help='The installation folder of Cygwin, which is required to build certain packages.')
        parser.add_argument('packages',
                            choices=self.available_packages,
                            nargs='*',
                            help='Select a sub-set of packages to build. ' +
                                 'If none are selected, all available packages will be built.')

        args = parser.parse_args()
        self.toolset = args.toolset

        toolset_prefix = self.toolset
        if self.toolset == 'msvc16':
            toolset_prefix = 'vc142'
        elif self.toolset == 'msvc15':
            toolset_prefix = 'vc141'

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
        self.keep_temporary_files = args.keep_temporary_files

        self.mirror = args.mirror[0]
        if not self.mirror.endswith('/'):
            self.mirror = self.mirror + '/'

        self.no_dependencies = args.no_dependencies
        if args.packages:
            self.packages = args.packages
        else:
            self.packages = self.available_packages
        install_prefix_folder = Path('{}-{}-{}'.format(
            self.target_platform,
            self.target_system,
            toolset_prefix))
        if self.host_system == 'windows':
            self.cygwin = Path(args.cygwin)
            if not self.cygwin.exists():
                raise RuntimeError('Cygwin must be installed.')
            if not (self.cygwin / 'bin' / 'make.exe').exists():
                raise RuntimeError('The GNU make Cygwin package must be installed.')
            if not (self.cygwin / 'bin' / 'link.exe').exists():
                raise RuntimeError('The binutils Cygwin package must be installed.')
            if not (self.cygwin / 'bin' / 'patch.exe').exists():
                raise RuntimeError('The patch Cygwin package must be installed.')
            self._patch = self.cygwin / 'bin' / 'patch.exe'

        if self.toolset == 'gcc':
            self.cmake_generator = 'Ninja'
        elif self.toolset == 'clang':
            self.cmake_generator = 'Ninja'
            # Clang may build against different standard libraries
            install_prefix_folder = Path('{}-{}-{}-{}'.format(
                self.target_platform,
                self.target_system,
                toolset_prefix,
                self.stdlib))
        elif self.toolset == 'msvc16':
            self.check_msvc()
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
                    self.toolset, self.target_platform)
        elif self.toolset == 'msvc15':
            self.check_msvc()
            if self.target_platform == 'x86_64':
                self.cmake_generator = 'Visual Studio 15 2017 Win64'
            elif self.target_platform == 'x86_32':
                self.cmake_generator = 'Visual Studio 15 2017'
            else:
                raise NotImplementedError(
                    'The selected toolset does not support building for the ' +
                    'selected target platform.',
                    self.toolset, self.target_platform)

        self.install_prefix = (Path.cwd() / '..' / install_prefix_folder).resolve()
        os.makedirs(self.install_prefix, mode=0o755, exist_ok=True)

        print('* Toolset: {}'.format(self.toolset))
        print('* Target System: {}'.format(self.target_system))
        print('* Target Platform: {} ({} bit)'.format(self.target_platform, self.target_platform_bits))
        if self.toolset == 'clang':
            print('* Standard Library: {}'.format(self.stdlib))
        print('* Install Prefix: {}'.format(self.install_prefix))

    def _load_package(self, package_modules, package_name):
        # Don't load a package twice
        for existing_package_name, existing_module in package_modules:
            if existing_package_name == package_name:
                return True

        # Dynamically import source file package.
        print('* Loading package ' + package_name)
        spec = importlib.util.spec_from_file_location(package_name,
                                                      package_name + '.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        package_modules.insert(0, (package_name, module))

        # If dependencies contains only a string pack it into a list.
        if isinstance(module.dependencies, str):
            module.dependencies = [module.dependencies]
        if not self.no_dependencies:
            # Recursively load all dependencies.
            for dependency in module.dependencies:
                self._load_package(package_modules, dependency)

    def run(self):
        package_modules = []

        # Dynamically recursively import all source file packages with dependencies.
        for package_name in self.packages:
            self._load_package(package_modules, package_name)

        # Sort packages by dependencies.
        change_package_order = True
        while change_package_order:
            change_package_order = False

            source_index = 0
            while source_index < len(package_modules) and not change_package_order:
                source_package_name, source_module = package_modules[source_index]

                target_index = 0
                for dependency in source_module.dependencies:
                    dependency_found = False
                    j = 0
                    while j < len(package_modules):
                        other_package_name, other_module = package_modules[j]
                        if dependency == other_package_name:
                            dependency_found = True
                            if source_index < j:
                                target_index = max(target_index, j)
                                change_package_order = True
                                break
                        j += 1
                    if not dependency_found and not self.no_dependencies:
                        raise RuntimeError('Cannot find dependency "{}" of package "{}"'.format(dependency,
                                                                                                source_package_name))
                if change_package_order:
                    # Because we first remove the element from the list the target index needs to be reduced by one.
                    package_modules.insert(target_index, package_modules.pop(source_index))
                    break
                source_index += 1

        print('* Build order:', end=' ')
        for package_name, module in package_modules:
            print(package_name, end=' ')
        print()

        pwd = os.getcwd()
        for package_name, module in package_modules:
            print('* Building package ' + package_name)
            try:
                module.cleanup(self)
                if module.prepare(self):
                    module.build(self)
            except Exception as ex:
                print(ex)
                input('Press ENTER to resume. You may use this pause to further investigate the error.')
                raise
            finally:
                os.chdir(pwd)
                if not self.keep_temporary_files:
                    module.cleanup(self)

    #
    def extract(self, filename: Path):
        filename = Path(filename)
        self._download_file(filename)
        Builder.check_file_exists(filename)
        try:
            print('* Extracting archive "{}" ...'.format(filename),
                  end=' ', flush=True)
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
    def apply_patch(self, args, filename: Path):
        filename = Path(filename)
        self._download_file(filename)

        patch_args = [self._patch]
        patch_args.extend(args)
        subprocess.run(patch_args, input=str.encode(Path(filename).resolve().read_text()))

    #
    def cmake(self, source_path="../", build_type=None, args=[], env_vars={}, install=False):
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

        if install:
            cmake_args = ['cmake', '--build', '.', '--target', 'install']
            if build_type:
                cmake_args.extend(['--config', build_type])
            subprocess.check_call(cmake_args, env=environment)

    #
    def configure(self, env_vars={}, args=[]):
        environment = self.setup_env(env_vars)
        configure_args = ['/bin/env', './configure',
                          '--prefix={}'.format(self.install_prefix)]
        configure_args.extend(args)
        subprocess.check_call(configure_args, env=environment)

    #
    def make(self, env_vars={}, args=[], env_as_args=False, parallel=True, install=False, verbose=False):
        environment = self.setup_env(env_vars)
        if self.host_system == 'windows':
            make_path = builder.cygwin / 'bin' / 'make.exe'
        else:
            make_path = 'make'
        make_args = [make_path, '-j{}'.format(multiprocessing.cpu_count() if parallel else 1)]
        if env_as_args:
            make_args.extend(['CC={}'.format(environment.get('CC', '')),
                              'CXX={}'.format(environment.get('CXX', '')),
                              'CFLAGS={}'.format(environment.get('CFLAGS', '')),
                              'CXXFLAGS={}'.format(environment.get('CXXFLAGS', ''))])
        if install:
            make_args.extend(['install',
                              "PREFIX='{}'".format(self.install_prefix)])
        if verbose:
            make_args.append('VERBOSE=1')
        make_args.extend(args)
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
                print('* Installing {} -> {}'.format(source_path.as_posix(),
                                                     destination_path.as_posix()))
                shutil.copyfile(source_path.as_posix(),
                                destination_path.as_posix())

    #
    @staticmethod
    def dos2unix(patterns=[]):
        for pattern in patterns:
            for filename in Path.cwd().glob(pattern):
                with open(filename.as_posix(), 'rb') as file:
                    file_data = file.read()
                file_data = file_data.replace(b'\r\n', b'\n')
                with open(filename.as_posix(), 'wb') as file:
                    file.write(file_data)

    #
    @staticmethod
    def check_file_exists(filename: Path):
        filename = Path(filename)
        if not filename.exists():
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT),
                                    filename)

    #
    @staticmethod
    def remove_folder(path: Path, exit_on_error=False):
        path = Path(path)
        if path.exists() and path.is_dir():
            try:
                print('* Removing folder {} ...'.format(path),
                      end=' ', flush=True)
                shutil.rmtree(path.as_posix())
                print('OK')
            except Exception as ex:
                print('FAILED')
                print(ex)
                if exit_on_error:
                    raise

    #
    def setup_env(self, env_vars={}):
        environment = os.environ.copy()

        # Eventually fix PWD variable
        if 'PWD' in environment:
            environment['PWD'] = Path.cwd().as_posix()

        env_var_separator = ';' if self.host_system == 'windows' else ':'

        include = ''
        if 'INCLUDE' in environment:
            include = environment['INCLUDE']
        environment['INCLUDE'] = include + env_var_separator + \
                                 (self.install_prefix / 'include').as_posix()

        lib = ''
        if 'LIB' in environment:
            lib = environment['LIB']
        environment['LIB'] = lib + env_var_separator + \
                             (self.install_prefix / 'lib').as_posix()

        if self.cygwin:
            path = ''
            if 'PATH' in environment:
                path = environment['PATH']
            environment['PATH'] = path + env_var_separator + \
                                  (self.cygwin / 'bin').as_posix()

        if self.toolset == 'gcc':
            environment['CC'] = 'gcc'
            environment['CXX'] = 'g++'
        elif self.toolset == 'clang':
            environment['CC'] = 'clang'
            environment['CXX'] = 'clang++'
            environment['CXXFLAGS'] = '{} -stdlib={}'.format(environment.get('CXXFLAGS', ''), self.stdlib)
        elif self.toolset.startswith('msvc'):
            environment['CC'] = 'cl'
            environment['CXX'] = 'cl'

        for key, value in env_vars.items():
            if key[0] == '+':
                key = key[1:]
                environment[key] = environment[key] + env_var_separator + value
            else:
                environment[key] = value

        environment['BUILD_PREFIX'] = self.install_prefix.as_posix()
        return environment

    def _check_msvc(self):
        try:
            output = subprocess.check_output(['cl.exe'], stderr=subprocess.STDOUT).decode().partition('\n')[0]
        except Exception:
            return False

        pattern = re.compile(
            'Microsoft \\(R\\) C/C\\+\\+ Optimizing Compiler Version (19).(2[0-9]).([0-9]+).([0-9]) for (x86|x64)')
        match = pattern.match(output)
        if not match:
            return False
        major = int(match.groups()[0])
        minor = int(match.groups()[1])
        # build = int(match.groups()[2])
        # patch = int(match.groups()[3])
        platform = match.groups()[4]

        if self.toolset == 'msvc16':
            if major != 19:
                return False
            if minor < 20 or minor >= 30:
                return False
        elif self.toolset == 'msvc15':
            if major != 19:
                return False
            if minor < 20 or minor >= 30:
                return False

        if self.target_platform == 'x86_64':
            if platform != 'x64':
                return False
        elif self.target_platform == 'x86_32':
            if platform != 'x86':
                return False
        else:
            raise NotImplementedError('Support for MSVC targeting ARM is not yet implemented.')

        return True

    def check_msvc(self):
        if not self._check_msvc():
            raise RuntimeError(
                'The Microsoft Visual Studio 16 (2019) C++ compiler is not available on PATH or setup incorrectly.\n' +
                'Please call this script from a properly setup command prompt.\n' +
                'If you don''t know how to do this, try to call the following command:\n' +
                '  call "c:\\Program Files (x86)\\Microsoft Visual Studio\\{}\\Professional\\VC\\Auxiliary\\' +
                'Build\\vcvarsall.bat" {}'.format(
                    '2019' if self.toolset == 'msvc16' else '2017',
                    'amd64' if self.target_platform == 'x86_64' else 'x86'))

    #
    @staticmethod
    def package_name_from_filename(filename):
        package_name, file_extension = os.path.splitext(os.path.basename(filename))
        return package_name

    #
    @staticmethod
    def _detect_cygwin():
        folder = Path('c:/cygwin64')
        if folder.exists():
            return folder
        folder = Path('c:/cygwin')
        if folder.exists():
            return folder
        return Path()

    #
    def _download_file(self, filename: Path):
        filename = Path(filename)
        if not filename.exists():
            try:
                url = '{}{}'.format(self.mirror, filename.name)
                print('* Downloading archive "{}" ...'.format(url),
                      end=' ', flush=True)
                urllib.request.urlretrieve(url, filename, Builder._show_download_progress)
            except Exception:
                print('FAILED')
                raise

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

    available_packages = []
    toolset = ''
    host_system = ''
    target_platform = ''
    target_platform_bits = 64
    target_system = ''
    stdlib = 'libstdc++'
    no_dependencies = False
    packages = []
    cmake_generator = 'Ninja'
    cmake_generator_platform = ''
    install_prefix = Path('.').resolve()
    mirror = 'https://boxie.eu/3rdparty/',
    keep_temporary_files = False
    cygwin = None
    _patch = 'patch'


if __name__ == "__main__":
    builder = Builder()
    builder.run()
