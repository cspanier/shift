#!/bin/bash

export CC=clang
export CXX=clang++
export HOST_COMPILER=clang
export TRIPLE=x86_64-linux-${HOST_COMPILER}
export CMAKE_GENERATOR=Ninja

./cmake.sh $@
