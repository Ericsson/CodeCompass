#!/bin/bash

# Compile dependencies that cannot be acquired via the official repositories

mkdir -p ${INSTALL_PATH}

## Compile Thrift
CXXFLAGS_OLD=$CXXFLAGS
LDFLAGS_OLD=$LDFLAGS
export CXXFLAGS="$CXXFLAGS -I/opt/homebrew/include"
export LDFLAGS="$LDFLAGS -L/opt/homebrew/lib"
export CXXFLAGS="$CXXFLAGS -Wno-error"

tar -xvf ${DOWNLOAD_PATH}/thrift-0.16.0.tar.gz
cd thrift-0.16.0
./configure --prefix=${INSTALL_PATH}/thrift --silent --without-python \
  --enable-libtool-lock --enable-tutorial=no --enable-tests=no      \
  --with-libevent --with-zlib --without-nodejs --without-lua        \
  --without-ruby --without-csharp --without-erlang --without-perl   \
  --without-php --without-php_extension --without-dart              \
  --without-haskell --without-go --without-rs --without-haxe        \
  --without-dotnetcore --without-d --without-qt4 --without-qt5      \
  --without-java --without-swift                                    \
  --with-openssl=/opt/homebrew/opt/openssl

make install -j $(nproc)

export CXXFLAGS=$CXXFLAGS_OLD
export LDFLAGS=$LDFLAGS_OLD

## Compile build2

sh "${DOWNLOAD_PATH}/install_latest_build2.sh" ${INSTALL_PATH}/build2

## Compile libodb runtime libraries
mkdir -p ${DOWNLOAD_PATH}/libodb
cd ${DOWNLOAD_PATH}/libodb
${INSTALL_PATH}/build2/bin/bpkg create --quiet --jobs $(nproc) cc \
  config.cxx=g++ \
  config.cc.coptions=-O3 \
  config.bin.rpath=${INSTALL_PATH}/odb/lib \
  config.install.root=${INSTALL_PATH}/odb

### Getting the source
${INSTALL_PATH}/build2/bin/bpkg add https://pkg.cppget.org/1/beta --trust-yes
${INSTALL_PATH}/build2/bin/bpkg fetch --trust-yes

### Building ODB runtime library
${INSTALL_PATH}/build2/bin/bpkg build libodb --yes
${INSTALL_PATH}/build2/bin/bpkg build libodb-sqlite --yes
${INSTALL_PATH}/build2/bin/bpkg build libodb-pgsql --yes
${INSTALL_PATH}/build2/bin/bpkg install --all --recursive

## Compile odb compiler
mkdir -p ${DOWNLOAD_PATH}/odb
cd ${DOWNLOAD_PATH}/odb
bpkg create --quiet --jobs $(nproc) cc \
  config.cxx=g++-13 \
  config.cc.poptions=-I/opt/homebrew/include \
  config.cc.coptions=-O3 \
  config.bin.rpath=${INSTALL_PATH}/odb/lib \
  config.install.root=${INSTALL_PATH}/odb

### Getting the source
bpkg add https://pkg.cppget.org/1/beta --trust-yes
bpkg fetch --trust-yes

### Building ODB Compiler
bpkg build odb --yes
bpkg install odb