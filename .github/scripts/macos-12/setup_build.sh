#!/bin/bash

# Install available dependencies from HomeBrew
brew install wget
brew install node
brew install sqlite
brew install cmake
brew install llvm@15
brew install gcc
brew install boost
brew install bison
brew install graphviz
brew install googletest
brew install libgit2
brew install libmagic
brew install openssl@3
brew install gnu-sed
brew install coreutils

# Place Bison on the PATH
echo "/opt/homebrew/opt/bison/bin" >> $GITHUB_PATH