#!/usr/bin/env bash

set -eux

git submodule add git@github.com:the-tcpdump-group/libpcap.git ./thirdparty/libpcap
git submodule add git@github.com:google/googletest.git ./thirdparty/gtest