#!/usr/bin/env bash

set -eux
set -o pipefail

CURRENT_DIR=$(cd $(dirname $0); pwd)
CPU_PROCESSOR_NUM=$(grep processor /proc/cpuinfo | wc -l)
JOB_NUM=$(expr "${CPU_PROCESSOR_NUM}")

function build()
{
    rm -rf build/
    mkdir build
    cd build
    cmake -DDOWNLOAD_THIRDPARTY=false -DCMAKE_VERBOSE_MAKEFILE=on  -DCMAKE_BUILD_TYPE=Debug ..
    make -j${JOB_NUM} && make install
    cd ${CURRENT_DIR}
}

function deps()
{
    git submodule init
    git submodule update
    rm -rf build/
    mkdir build
    cd build
    cmake -DDOWNLOAD_THIRDPARTY=true -DCMAKE_VERBOSE_MAKEFILE=on  -DCMAKE_BUILD_TYPE=Debug ..
    make -j${JOB_NUM}
    cd ${CURRENT_DIR}
}

function all()
{
    deps
    build
}

function usage()
{
    echo "./build_debug.sh [-deps|-build|-all]"
    echo "-deps for download third party"
    echo "-build for build and install this project"
    echo "-all for build and install deps and this project"
}

if [ $1 = "-deps" ]
then
    deps
elif [ $1 = "-build" ]
then
    build
elif [ $1 = "-all" ]
then
    all
else
    usage
fi
