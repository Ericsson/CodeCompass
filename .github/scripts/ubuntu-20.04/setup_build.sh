#!/bin/bash

# Add official LLVM repositories
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-15 main" | sudo tee /etc/apt/sources.list.d/llvm.list
echo "deb-src http://apt.llvm.org/focal/ llvm-toolchain-focal-15 main" | sudo tee -a /etc/apt/sources.list.d/llvm.list
sudo apt-get update

# Install required packages for CodeCompass build
sudo apt-get install -y git cmake make g++ libboost-all-dev \
  llvm-15-dev clang-15 libclang-15-dev odb \
  libodb-dev default-jdk libssl-dev \
  libgraphviz-dev libmagic-dev libgit2-dev ctags doxygen libgtest-dev npm libldap2-dev
