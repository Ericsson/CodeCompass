#!/bin/bash

# Install required packages for CodeCompass build
sudo apt-get install --yes \
  git cmake make g++ libboost-all-dev \
  llvm-15-dev clang-15 libclang-15-dev \
  gcc-11-plugin-dev thrift-compiler libthrift-dev \
  default-jdk libssl-dev libgraphviz-dev libmagic-dev libgit2-dev exuberant-ctags doxygen \
  libldap2-dev libgtest-dev

wget https://packages.microsoft.com/config/ubuntu/22.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
sudo apt-get update
sudo apt-get install -y apt-transport-https
sudo apt-get update
sudo apt-get install -y dotnet-sdk-6.0