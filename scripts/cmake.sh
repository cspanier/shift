#!/bin/bash

export selfpath="$( cd "$(dirname "$0")" ; pwd -P )"

cmakefunc()
{
  echo "Do you wish to use include-what-you-use?"
  select yn in "Yes" "No"; do
    case $yn in
      Yes )
        export iwyu="$(which include-what-you-use);-Xiwyu;--max_line_length=800;-Xiwyu;--prefix_header_includes=add;-Xiwyu;--mapping_file=${selfpath}/iwyu-shift.imp"
        break;;
      No )
        export iwyu=
        break;;
    esac
  done

  export BUILD_FOLDER_NAME="build-${TRIPLE,,}-${BUILD_TYPE,,}"
  if [[ ! -e ./${BUILD_FOLDER_NAME} ]]; then
    mkdir ./${BUILD_FOLDER_NAME}
  fi
  cd ./${BUILD_FOLDER_NAME}
  if [[ -e CMakeCache.txt ]]; then
    rm CMakeCache.txt
  fi
  cmake -G "${CMAKE_GENERATOR}" .. \
    -DCMAKE_C_COMPILER:STRING=$(which ${CC}) \
    -DCMAKE_CXX_COMPILER:STRING=$(which ${CXX}) \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_PREFIX_PATH:PATH="/usr;${PWD}/../../3rdparty/${TRIPLE,,}-${HOST_COMPILER,,}" \
    -DCMAKE_INSTALL_PREFIX:PATH="${PWD}/../production" \
    -DCMAKE_TOOLCHAIN_FILE:FILEPATH="${PWD}/../cmake/Toolchain-${TRIPLE}.cmake" \
    -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE="${iwyu}" \
    -DSHIFT_DEBUG:BOOL=OFF \
    $*

  return 0
}

pushd $PWD > /dev/null
cd $(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../

export BUILD_TYPE=$1
if [[ "${BUILD_TYPE}" != "Debug" && \
      "${BUILD_TYPE}" != "MinSizeRel" && \
      "${BUILD_TYPE}" != "Release" && \
      "${BUILD_TYPE}" != "RelWithDebInfo" ]]; then
  echo "USAGE: $0 (Debug|MinSizeRel|Release|RelWithDebInfo) [CMake Args...]"
  exit 1;
fi

if [[ "${HOST_COMPILER}" == "" ]]; then
  echo "HOST_COMPILER environment variable not set."
  return 1
fi
if [[ "${TRIPLE}" == "" ]]; then
  echo "TRIPLE environment variable not set."
  return 1
fi
if [[ "${CMAKE_GENERATOR}" == "" ]]; then
  echo "CMAKE_GENERATOR environment variable not set."
  return 1
fi

cmakefunc ${@:2}

popd > /dev/null
