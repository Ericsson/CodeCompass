#!/bin/bash

# Install required packages for CodeCompass runtime
sudo apt-get install -y git cmake make g++ graphviz \
  libboost-filesystem1.71.0 libboost-log1.71.0 libboost-program-options1.71.0 \
  libllvm11 clang-11 libclang1-11 libthrift-0.16.0 default-jre libssl1.1 libmagic1 \
  libgit2-28 ctags googletest libldap-2.4-2