#!/bin/bash

# Compile dependencies that cannot be acquired via the official repositories

mkdir -p ${INSTALL_PATH}

## Compile Thrift
tar -xvf ${DOWNLOAD_PATH}/thrift-0.16.0.tar.gz
cd thrift-0.16.0
./configure --prefix=${INSTALL_PATH}/thrift --silent --without-python \
  --enable-libtool-lock --enable-tutorial=no --enable-tests=no      \
  --with-libevent --with-zlib --without-nodejs --without-lua        \
  --without-ruby --without-csharp --without-erlang --without-perl   \
  --without-php --without-php_extension --without-dart              \
  --without-haskell --without-go --without-rs --without-haxe        \
  --without-dotnetcore --without-d --without-qt4 --without-qt5      \
  --without-java --without-swift

make install -j $(nproc)