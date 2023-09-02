#!/bin/bash
set -x
set -e

#####################
# Build CodeCompass #
#####################

export PATH=$DEPS_INSTALL_BUILD_DIR/gcc-install/bin\
:$DEPS_INSTALL_BUILD_DIR/cmake-install/bin\
:$DEPS_INSTALL_RUNTIME_DIR/libmagic-install/bin\
:$DEPS_INSTALL_RUNTIME_DIR/odb-install/bin\
:$DEPS_INSTALL_RUNTIME_DIR/thrift-install/bin\
:$DEPS_INSTALL_RUNTIME_DIR/jdk-install/bin\
:$DEPS_INSTALL_RUNTIME_DIR/python-install/bin\
:$DEPS_INSTALL_RUNTIME_DIR/node-install/bin\
:$PATH

export CPLUS_INCLUDE_PATH=$DEPS_INSTALL_BUILD_DIR/gtest-install/include\
:$DEPS_INSTALL_RUNTIME_DIR/libgit2-install/include\
:$DEPS_INSTALL_RUNTIME_DIR/libmagic-install/include\
:$DEPS_INSTALL_RUNTIME_DIR/graphviz-install/include\
:$DEPS_INSTALL_RUNTIME_DIR/boost-install/include\
:$DEPS_INSTALL_RUNTIME_DIR/openssl-install/include

export C_INCLUDE_PATH=$DEPS_INSTALL_BUILD_DIR/gtest-install/include\
:$DEPS_INSTALL_RUNTIME_DIR/libgit2-install/include\
:$DEPS_INSTALL_RUNTIME_DIR/libmagic-install/include\
:$DEPS_INSTALL_RUNTIME_DIR/graphviz-install/include\
:$DEPS_INSTALL_RUNTIME_DIR/boost-install/include\
:$DEPS_INSTALL_RUNTIME_DIR/openssl-install/include

export LIBRARY_PATH=$DEPS_INSTALL_RUNTIME_DIR/graphviz-install/lib\
:$DEPS_INSTALL_RUNTIME_DIR/libmagic-install/lib\
:$DEPS_INSTALL_RUNTIME_DIR/libgit2-install/lib\
:$DEPS_INSTALL_RUNTIME_DIR/libgit2-install/lib64\
:$DEPS_INSTALL_RUNTIME_DIR/openssl-install/lib
# Note: libgit2-install/lib required on Ubuntu; libgit2-install/lib64 on SUSE

export LDFLAGS="-Wl,-rpath-link,$DEPS_INSTALL_RUNTIME_DIR/openssl-install/lib "\
"-Wl,-rpath-link,$DEPS_INSTALL_RUNTIME_DIR/odb-install/lib "\
"-Wl,-rpath-link,$DEPS_INSTALL_RUNTIME_DIR/postgresql-install/lib"
# Note: build2 compile libpg in odb-install/lib as well;
#       otherwise version in postgresql-install/lib is used

export LD_LIBRARY_PATH=$DEPS_INSTALL_RUNTIME_DIR/odb-install/lib\
:$DEPS_INSTALL_RUNTIME_DIR/python-install/lib\
:$LD_LIBRARY_PATH

export CMAKE_PREFIX_PATH=$DEPS_INSTALL_RUNTIME_DIR/libgit2-install\
:$DEPS_INSTALL_RUNTIME_DIR/odb-install\
:$DEPS_INSTALL_RUNTIME_DIR/thrift-install\
:$DEPS_INSTALL_RUNTIME_DIR/boost-install\
:$DEPS_INSTALL_RUNTIME_DIR/llvm-install\
:$DEPS_INSTALL_RUNTIME_DIR/python-install\
:$DEPS_INSTALL_RUNTIME_DIR/openldap-install

export GTEST_ROOT=$DEPS_INSTALL_BUILD_DIR/gtest-install

export CXX=$DEPS_INSTALL_BUILD_DIR/gcc-install/bin/g++
export CC=$DEPS_INSTALL_BUILD_DIR/gcc-install/bin/gcc

mkdir -p $CC_BUILD_DIR
cd $CC_BUILD_DIR
cmake $CC_SRC_DIR \
  -DCMAKE_INSTALL_PREFIX=$CC_INSTALL_DIR \
  -DDATABASE=pgsql \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DWITH_AUTH="plain;ldap" \
  -DLLVM_DIR=$DEPS_INSTALL_RUNTIME_DIR/llvm/lib/cmake/llvm/ \
  -DClang_DIR=$DEPS_INSTALL_RUNTIME_DIR/llvm/lib/cmake/clang/
make install --quiet --jobs $(nproc)
