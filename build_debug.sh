#!/usr/bin/env bash

set -eux

rm -rf build/
mkdir build
cd build
cmake -DDOWNLOAD_THIRDPARTY=false -DCMAKE_VERBOSE_MAKEFILE=on  -DCMAKE_BUILD_TYPE=Debug ..
make -j2