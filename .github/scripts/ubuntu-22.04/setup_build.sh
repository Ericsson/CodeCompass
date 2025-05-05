#!/bin/bash

# Install required packages for CodeCompass build
sudo apt install git cmake make g++ libboost-all-dev \
  llvm-15-dev clang-15 libclang-15-dev \
  gcc-11-plugin-dev thrift-compiler libthrift-dev \
  default-jdk libssl-dev libgraphviz-dev libmagic-dev libgit2-dev exuberant-ctags doxygen \
  libldap2-dev libgtest-dev