#!/bin/bash

if [[ "${1}" == "" ]]; then
  echo "Syntax: ${0} (build-folder)"
  exit 1
fi
if [[ ! -f "${1}/compile_commands.json" ]]; then
  echo "Cannot find compile_commands.json file in ${1}."
  exit 0
fi

export BASE_DIR="$(realpath $(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/..)"
for source in $(find ${BASE_DIR} -name "*.cpp"); do
  echo "Processing ${source}..."
  clang-tidy \
    -p "${1}" \
    -header-filter=shift/ \
    -config="{Checks: '-*,
      modernize-*,
      -modernize-loop-convert,
      -modernize-replace-random-shuffle,
      -modernize-shrink-to-fit,
      -modernize-use-equals-default,
      readability-*,
      -readability-braces-around-statements,
      -readability-else-after-return,
      -readability-magic-numbers'}" \
    -fix \
    -format-style=file \
    "${source}"
  clang-tidy \
    -p "${1}" \
    -header-filter=shift/ \
    -config="{Checks: '-*,modernize-*'}" \
    "${source}"
done
