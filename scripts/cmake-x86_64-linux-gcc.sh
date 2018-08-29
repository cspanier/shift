#!/bin/bash

export CC=gcc
export CXX=g++
export HOST_COMPILER=gcc
export TRIPLE=x86_64-linux-${HOST_COMPILER}
export CMAKE_GENERATOR=Ninja

./cmake.sh $@
