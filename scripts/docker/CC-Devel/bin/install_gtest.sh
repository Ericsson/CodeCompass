#!/usr/bin/env bash

pushd /usr/src/gtest
cmake .
make
cp libgtest.a libgtest_main.a /usr/lib
popd

