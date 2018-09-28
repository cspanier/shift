#!/bin/bash

for i in $(find shift/ -name "*.h" -o -name "*.cpp"); do
	echo ${i}
	clang-format -style=file -i "${i}"
done
