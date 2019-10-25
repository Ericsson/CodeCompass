#!/usr/bin/env bash

# This script will run inside the Docker container. This will build CodeCompass
# using the following environment variables:
#
# DATABASE - pgsql (by default) or sqlite
# BUILD_TYPE - Release (by default) or Debug

# DON'T MODIFY THE REST OF THIS SCRIPT UNLESS YOU KNOW WHAT YOU'RE DOING!

export PATH=/opt/thrift/bin:$PATH
export CMAKE_PREFIX_PATH=/opt/thrift

BUILD_DIR=/CodeCompass/build
INSTALL_DIR=/CodeCompass/install
SOURCE_DIR=/CodeCompass/CodeCompass

if [ ! -d $BUILD_DIR ]; then
  mkdir $BUILD_DIR

  cd $BUILD_DIR
  cmake \
    $SOURCE_DIR \
    -DDATABASE=$DATABASE \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
fi

cd $BUILD_DIR
make $@
