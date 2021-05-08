#!/usr/bin/env bash

# This script will run inside the Docker container. This will build CodeCompass
# using the following environment variables:
#
# DATABASE - sqlite (by default) or psql
# BUILD_TYPE - Release (by default) or Debug

# DON'T MODIFY THE REST OF THIS SCRIPT UNLESS YOU KNOW WHAT YOU'RE DOING!

mkdir -p $BUILD_DIR
mkdir -p $TEST_WORKSPACE

cd $BUILD_DIR
cmake \
  $SOURCE_DIR \
  -DDATABASE=$DATABASE \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DTEST_DB=$TEST_DB

make -C $BUILD_DIR $@
