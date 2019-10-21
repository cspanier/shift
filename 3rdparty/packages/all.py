#!/usr/bin/env python3

import sys
from build import Builder

package_name = Builder.package_name_from_filename(__file__)
dependencies = ('zlib-1.2.11',
                'bzip2-1.0.8',
                'jpeg-9c',
                'zstd-be3bd70',
                'tiff-4.0.10',
                'icu4c-65_1',
                'boost_1_71_0')


def prepare(builder):
    return True


def build(builder):
    return True


def cleanup(builder):
    return True


if __name__ == "__main__":
    print('You must not call this script directly.')
    sys.exit(1)
