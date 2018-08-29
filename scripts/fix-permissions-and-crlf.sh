#!/bin/bash
export IFS=$'\n'

pushd $PWD > /dev/null
cd $(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../

find . -type d \
	-not -path "./.git/*" \
	-print0 | while read -r -d '' file; do
  chmod 755 $file
done
find . -type f \
	-not -path "./.git/*" \
	-print0 | while read -r -d '' file; do
  chmod 644 $file
done
find . -type f -name "*.sh" \
	-not -path "./.git/*" \
	-print0 | while read -r -d '' file; do
  chmod 755 $file
done
find . -type f \
	-name "*.cpp" -o \
	-name "*.h" -o \
	-name "*.sh" -o \
	-name "*.cmd" -o \
	-name "CMakeLists.txt" \
	-not -path "./.git/*" \
	-print0 | while read -r -d '' file; do
  dos2unix -q $file
  touch $file
done

popd > /dev/null
