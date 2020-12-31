#!/bin/bash

#This script is intended to run inside the testing container.
#DO NOT use it on it's own.

set -euxo pipefail

function install_fossa {
apt-get update 
apt-get --yes install --no-install-recommends \
    cmake make \
    default-jdk \
    ctags \
    gcc-9 gcc-9-plugin-dev g++-9 \
    libboost-filesystem-dev \
    libboost-log-dev \
    libboost-program-options-dev \
    libboost-regex-dev \
    libgit2-dev \
    libgraphviz-dev \
    libgtest-dev \
    libmagic-dev \
    libsqlite3-dev \
    libssl-dev \
    llvm-10 clang-10 llvm-10-dev libclang-10-dev \
    npm \
    thrift-compiler libthrift-dev \
    odb libodb-sqlite-dev libodb-pgsql-dev && \
    ln -s /usr/bin/gcc-9 /usr/bin/gcc && \
    ln -s /usr/bin/g++-9 /usr/bin/g++

# Build GTest.
cd /usr/src/googletest && \
mkdir build && \
cd build && \
cmake .. && \
make install && \
cd / && \
rm -rf /usr/src/googletest/build

}

function install_beaver {
apt-get update 
apt-get --yes install --no-install-recommends \
  git cmake make g++ gcc-7-plugin-dev libboost-all-dev \
  llvm-10-dev clang-10 libclang-10-dev \
  default-jdk libssl1.0-dev libgraphviz-dev libmagic-dev libgit2-dev ctags \
  libgtest-dev npm libsqlite3-dev wget curl

#Install odb
wget https://download.build2.org/0.13.0/build2-install-0.13.0.sh
sh build2-install-0.13.0.sh --yes --trust yes "/opt/build2"
export PATH="/opt/build2/bin:$PATH" 
mkdir /tmp/odb_build
cd /tmp/odb_build
bpkg create --quiet --jobs $(nproc) cc \
  config.cxx=g++ \
  config.cc.coptions=-O3 \
  config.bin.rpath=/opt/odb/lib \
  config.install.root=/opt/odb

bpkg add https://pkg.cppget.org/1/beta --trust-yes
bpkg fetch --trust-yes

# Building odb
bpkg build odb --yes
bpkg build libodb --yes
bpkg build libodb-sqlite --yes
#bpkg build libodb-pgsql --yes
bpkg install --all --recursive

#Build thrift
wget "http://www.apache.org/dyn/mirrors/mirrors.cgi?action=download&filename=thrift/0.13.0/thrift-0.13.0.tar.gz" \
  -O thrift-0.13.0.tar.gz
tar -xvf ./thrift-0.13.0.tar.gz
cd thrift-0.13.0

./configure --prefix=/opt/thrift --silent --without-python          \
  --enable-libtool-lock --enable-tutorial=no --enable-tests=no      \
  --with-libevent --with-zlib --without-nodejs --without-lua        \
  --without-ruby --without-csharp --without-erlang --without-perl   \
  --without-php --without-php_extension --without-dart              \
  --without-haskell --without-go --without-rs --without-haxe        \
  --without-dotnetcore --without-d --without-qt4 --without-qt5      \
  --without-java

make install -j $(nproc)


#build gtest
mkdir /tmp/gtest -p
cp -R /usr/src/googletest/* /tmp/gtest

cd /tmp/gtest
mkdir build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX=/opt/gtest
make install -j $(nproc)

rm -rf build
}

if [[ $1 == *"20.04"* ]]; then
    install_fossa    
elif [[ $1 == *"18.04"* ]]; then
    install_beaver
else
    exit
fi
