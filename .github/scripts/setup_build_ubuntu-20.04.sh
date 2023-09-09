#!/bin/bash

# Install required packages for CodeCompass build
sudo apt-get install -y git cmake make g++ libboost-all-dev llvm-10-dev clang-10 \
  libclang-10-dev odb libodb-dev thrift-compiler libthrift-dev default-jdk libssl-dev \
  libgraphviz-dev libmagic-dev libgit2-dev ctags doxygen libgtest-dev npm libldap2-dev libyaml-cpp-dev
