#!/bin/bash

# ToDo: Check if Google Breakpad tool dump_syms is available
if [[ ! -x $(command -v dump_syms) ]]; then
  echo "Error: dump_syms from Google Breakpad is either not installed, or not in PATH."
  exit 1
fi

print_help()
{
  echo "${0} (path-to-executable) (symbols-base-path)"
  return 0
}

if [[ "${2}" == "" ]]; then
  print_help
  exit 1
fi

export input=${1}
export base_path=${2}

if [[ ! -f ${input} ]]; then
  echo "Error: Cannot find input executable \"${input}\""
  print_help
  exit 1
fi
export output_path=${base_path}/symbols/$(basename ${input})/$(dump_syms -i ${input} | \
  head -n1 | cut --delimiter=' ' -f4)
export output_file=${output_path}/$(basename ${input}).sym
mkdir -p "${output_path}"
if [[ ! -f ${output_file} ]]; then
  dump_syms ${input} > "${output_file}"
fi
