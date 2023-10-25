#!/bin/bash
set -x
set -e

mkdir -p $PACKAGES_DIR

#######
# GCC #
#######

if [ ! -f $DEPS_INSTALL_BUILD_DIR/gcc-install/bin/g++ ]; then
  cd $PACKAGES_DIR
  wget --no-verbose --no-clobber http://robotlab.itk.ppke.hu/gcc/releases/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz
  tar -xf gcc-$GCC_VERSION.tar.xz
  cd gcc-$GCC_VERSION
  ./contrib/download_prerequisites
  mkdir gcc-build
  cd gcc-build
  $PACKAGES_DIR/gcc-$GCC_VERSION/configure \
    --quiet \
    --enable-languages=c,c++ \
    --disable-multilib \
    --prefix=$DEPS_INSTALL_BUILD_DIR/gcc-install
  make --quiet --jobs $(nproc)
  make install --quiet
  rm -f $PACKAGES_DIR/gcc-$GCC_VERSION.tar.xz
else
  echo "Found GCC in cache."
fi
export PATH=$DEPS_INSTALL_BUILD_DIR/gcc-install/bin:$PATH
export CXX=$DEPS_INSTALL_BUILD_DIR/gcc-install/bin/g++
export CC=$DEPS_INSTALL_BUILD_DIR/gcc-install/bin/gcc

#########
# CMake #
#########

if [ ! -f $DEPS_INSTALL_BUILD_DIR/cmake-install/bin/cmake ]; then
  cd $PACKAGES_DIR
  wget --no-verbose --no-clobber https://github.com/Kitware/CMake/releases/download/v3.19.0/cmake-3.19.0-Linux-x86_64.tar.gz
  tar -xf cmake-3.19.0-Linux-x86_64.tar.gz -C $DEPS_INSTALL_BUILD_DIR
  rm -rf cmake-3.19.0-Linux-x86_64.tar.gz

  cd $DEPS_INSTALL_BUILD_DIR
  mv cmake-3.19.0-Linux-x86_64 cmake-install
fi
export PATH=$DEPS_INSTALL_BUILD_DIR/cmake-install/bin:$PATH

######
# m4 #
######

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://ftp.gnu.org/gnu/m4/m4-1.4.19.tar.gz
tar -xf m4-1.4.19.tar.gz
cd m4-1.4.19
./configure --quiet --prefix=$DEPS_INSTALL_BUILD_DIR/m4-install
make install --quiet --jobs $(nproc)
rm -rf $PACKAGES_DIR/m4-1.4.19.tar.gz

###########
# libtool #
###########

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://quantum-mirror.hu/mirrors/pub/gnu/libtool/libtool-2.4.6.tar.gz
tar -xf libtool-2.4.6.tar.gz
cd libtool-2.4.6
PATH=$DEPS_INSTALL_BUILD_DIR/m4-install/bin:$PATH \
./configure --quiet --prefix=$DEPS_INSTALL_RUNTIME_DIR/libtool-install
make install --quiet --jobs $(nproc)
rm -rf $PACKAGES_DIR/libtool-2.4.6.tar.gz

###########
# openssl #
###########

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://www.openssl.org/source/openssl-1.1.1h.tar.gz
tar -xf openssl-1.1.1h.tar.gz
cd openssl-1.1.1h
./config \
  --prefix=$DEPS_INSTALL_RUNTIME_DIR/openssl-install \
  --openssldir=/etc/ssl
make install --quiet # must run as single core job
rm -f $PACKAGES_DIR/openssl-1.1.1h.tar.gz

############
# libmagic #
############

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://skynet.elte.hu/libmagic-5.18.tar.gz
tar -xf libmagic-5.18.tar.gz
cd libmagic-5.18/

./configure --quiet --prefix=$DEPS_INSTALL_RUNTIME_DIR/libmagic-install
make install --quiet --jobs $(nproc)
rm -f $PACKAGES_DIR/libmagic-5.18.tar.gz

###########
# libcutl #
###########
# note: needed for ODB 2.4.0

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://www.codesynthesis.com/download/libcutl/1.10/libcutl-1.10.0.tar.gz
tar -xf libcutl-1.10.0.tar.gz
cd libcutl-1.10.0

./configure --quiet --prefix=$DEPS_INSTALL_BUILD_DIR/libcutl-install
make install --quiet --jobs $(nproc)
rm -f $PACKAGES_DIR/libcutl-1.10.0.tar.gz

##############
# PostgreSQL #
##############

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://ftp.postgresql.org/pub/source/v12.16/postgresql-12.16.tar.gz
tar -xf postgresql-12.16.tar.gz
cd postgresql-12.16

./configure \
  --quiet \
  --prefix=$DEPS_INSTALL_RUNTIME_DIR/postgresql-install \
  --without-readline \
  --without-zlib

make install --quiet --jobs $(nproc)
rm -f $PACKAGES_DIR/postgresql-12.16.tar.gz

###############
# odb, libodb #
###############

if [ ! -f $DEPS_INSTALL_RUNTIME_DIR/odb-install/bin/odb ]; then
  if [[ $ODB_VERSION == "2.5.0" ]]; then
    # build2
    cd $PACKAGES_DIR
    wget --no-verbose --no-clobber https://download.build2.org/0.16.0/build2-install-0.16.0.sh
    sh build2-install-0.16.0.sh --yes --trust yes --jobs $(nproc) $PACKAGES_DIR/build2-install
    export PATH=$PACKAGES_DIR/build2-install/bin:$PATH

    # odb, libodb
    cd $PACKAGES_DIR
    mkdir odb-build
    cd odb-build
    bpkg create --quiet --jobs $(nproc) cc \
      config.cxx=g++ \
      config.cc.coptions=-O3 \
      config.bin.rpath_link=$DEPS_INSTALL_RUNTIME_DIR/odb-install/lib \
      config.install.root=$DEPS_INSTALL_RUNTIME_DIR/odb-install

    bpkg add https://pkg.cppget.org/1/beta --trust-yes
    bpkg fetch --trust-yes

    bpkg build odb --yes --quiet --jobs $(nproc)
    bpkg build libodb --yes --quiet --jobs $(nproc)
    bpkg build libodb-sqlite --yes --quiet --jobs $(nproc)
    bpkg build libodb-pgsql --yes --quiet --jobs $(nproc)
    bpkg install --all --recursive --quiet --jobs $(nproc)

    rm -f $PACKAGES_DIR/build2-toolchain-0.16.0.tar.xz
    rm -f $PACKAGES_DIR/build2-install-0.16.0.sh
  elif [[ $ODB_VERSION == "2.4.0" ]]; then
    # odb
    cd $PACKAGES_DIR
    wget --no-verbose --no-clobber https://www.codesynthesis.com/download/odb/2.4/odb-2.4.0.tar.gz

    tar -xf odb-2.4.0.tar.gz
    cd odb-2.4.0

    ./configure --quiet \
      --prefix=$DEPS_INSTALL_RUNTIME_DIR/odb-install \
      --with-libcutl=$PACKAGES_DIR/libcutl-1.10.0 # build dir
    make install --quiet --jobs $(nproc)
    rm -f $PACKAGES_DIR/odb-2.4.0.tar.gz


    # libodb
    cd $PACKAGES_DIR
    wget --no-verbose --no-clobber https://www.codesynthesis.com/download/odb/2.4/libodb-2.4.0.tar.gz

    tar -xf libodb-2.4.0.tar.gz
    cd libodb-2.4.0

    ./configure --quiet \
      --prefix=$DEPS_INSTALL_RUNTIME_DIR/odb-install
    make install --quiet --jobs $(nproc)
    rm -f $PACKAGES_DIR/libodb-2.4.0.tar.gz


    # libodb-pgsql
    cd $PACKAGES_DIR
    wget --no-verbose --no-clobber https://www.codesynthesis.com/download/odb/2.4/libodb-pgsql-2.4.0.tar.gz

    tar -xf libodb-pgsql-2.4.0.tar.gz
    cd libodb-pgsql-2.4.0

    CXXFLAGS="$CXXFLAGS -I$DEPS_INSTALL_RUNTIME_DIR/postgresql-install/include" \
    LDFLAGS="$LDFLAGS -L$DEPS_INSTALL_RUNTIME_DIR/postgresql-install/lib" \
    ./configure \
      --prefix=$DEPS_INSTALL_RUNTIME_DIR/odb-install \
      --with-libodb=$PACKAGES_DIR/libodb-2.4.0 # build dir
    make install --quiet --jobs $(nproc)
    rm -f $PACKAGES_DIR/libodb-pgsql-2.4.0.tar.gz
  else
    echo "ERROR: Not support ODB version."
    exit 1
  fi
else
  echo "Found ODB in cache."
fi

############
# GraphViz #
############

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://graphviz.gitlab.io/pub/graphviz/stable/SOURCES/graphviz.tar.gz
tar -xf graphviz.tar.gz
cd graphviz-2.40.1

./configure --quiet --prefix=$DEPS_INSTALL_RUNTIME_DIR/graphviz-install
make install --quiet --jobs $(nproc)
rm -f $PACKAGES_DIR/graphviz.tar.gz

##########
# Python #
##########

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://www.python.org/ftp/python/3.9.0/Python-3.9.0.tar.xz
tar -xf Python-3.9.0.tar.xz
cd Python-3.9.0

# needed for Python compile AND run
export LD_LIBRARY_PATH=$DEPS_INSTALL_RUNTIME_DIR/openssl-install/lib:$LD_LIBRARY_PATH

./configure \
  --quiet \
  --prefix=$DEPS_INSTALL_RUNTIME_DIR/python-install \
  --with-openssl=$DEPS_INSTALL_RUNTIME_DIR/openssl-install \
  --enable-optimizations
make install --quiet --jobs $(nproc)
rm -f $PACKAGES_DIR/Python-3.9.0.tar.xz

export PATH=$DEPS_INSTALL_RUNTIME_DIR/python-install/bin:$PATH

##############
# LLVM/Clang #
##############

if [ ! -f $DEPS_INSTALL_RUNTIME_DIR/llvm-install/bin/clang ]; then
  cd $PACKAGES_DIR
  wget --no-verbose --no-clobber https://github.com/llvm/llvm-project/archive/llvmorg-11.1.0.tar.gz
  tar -xf llvmorg-11.1.0.tar.gz
  mv llvm-project-llvmorg-11.1.0 llvm-project
  mkdir llvm-project/build
  cd llvm-project/build

  cmake ../llvm \
    -G "Unix Makefiles" \
    -DLLVM_ENABLE_PROJECTS=clang \
    -DCMAKE_INSTALL_PREFIX=$DEPS_INSTALL_RUNTIME_DIR/llvm-install \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RTTI=ON

  make install --quiet --jobs $(nproc)
  rm -f $PACKAGES_DIR/llvmorg-11.1.0.tar.gz
else
  echo "Found LLVM/Clang in cache."
fi

###########
# libgit2 #
###########

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://github.com/libgit2/libgit2/releases/download/v1.1.0/libgit2-1.1.0.tar.gz
tar -xf libgit2-1.1.0.tar.gz
mkdir libgit2-1.1.0/build
cd libgit2-1.1.0/build

cmake .. \
  -DCMAKE_INSTALL_PREFIX=$DEPS_INSTALL_RUNTIME_DIR/libgit2-install \
  -DOPENSSL_ROOT_DIR=$DEPS_INSTALL_RUNTIME_DIR/openssl-install
make install --quiet --jobs $(nproc)
rm -f $PACKAGES_DIR/libgit2-1.1.0.tar.gz

#########
# ctags #
#########

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://github.com/universal-ctags/ctags/archive/p5.9.20201129.0.tar.gz \
  -O ctags-p5.9.20201129.0.tar.gz
tar -xf ctags-p5.9.20201129.0.tar.gz
cd ctags-p5.9.20201129.0
./autogen.sh
./configure --quiet --prefix=$DEPS_INSTALL_RUNTIME_DIR/ctags-install
make install --quiet --jobs $(nproc)
rm -f $PACKAGES_DIR/ctags-p5.9.20201129.0.tar.gz

#########
# Boost #
#########

if [ ! -f $DEPS_INSTALL_RUNTIME_DIR/boost-install/lib/libboost_program_options.so ]; then
  cd $PACKAGES_DIR
  wget --no-verbose --no-clobber https://boostorg.jfrog.io/artifactory/main/release/1.74.0/source/boost_1_74_0.tar.gz
  tar -xf boost_1_74_0.tar.gz
  cd boost_1_74_0

  ./bootstrap.sh \
    --prefix=$DEPS_INSTALL_RUNTIME_DIR/boost-install \
    --with-python=$DEPS_INSTALL_RUNTIME_DIR/python-install/bin/python
  ./b2 -j $(nproc) install

  rm -f $PACKAGES_DIR/boost_1_74_0.tar.gz
else
  echo "Found Boost in cache."
fi

##########
# Thrift #
##########

cd $PACKAGES_DIR
wget --no-verbose --no-clobber http://archive.apache.org/dist/thrift/0.16.0/thrift-0.16.0.tar.gz
tar -xf thrift-0.16.0.tar.gz
cd thrift-0.16.0

CXXFLAGS="$CXXFLAGS -I$DEPS_INSTALL_RUNTIME_DIR/boost-install/include" \
LDFLAGS="$LDFLAGS -Wl,-rpath-link,$DEPS_INSTALL_RUNTIME_DIR/openssl-install/lib" \
./configure \
  --quiet \
  --prefix=$DEPS_INSTALL_RUNTIME_DIR/thrift-install \
  --with-boost=$DEPS_INSTALL_RUNTIME_DIR/boost-install \
  --with-openssl=$DEPS_INSTALL_RUNTIME_DIR/openssl-install \
  --without-python

make install --quiet --jobs $(nproc)
rm -f $PACKAGES_DIR/thrift-0.16.0.tar.gz

########
# Java #
########

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://download.java.net/java/GA/jdk15.0.1/51f4f36ad4ef43e39d0dfdbaf6549e32/9/GPL/openjdk-15.0.1_linux-x64_bin.tar.gz
tar -xf openjdk-15.0.1_linux-x64_bin.tar.gz -C $DEPS_INSTALL_RUNTIME_DIR
rm -f openjdk-15.0.1_linux-x64_bin.tar.gz

cd $DEPS_INSTALL_RUNTIME_DIR
mv jdk-15.0.1 jdk-install

export PATH=$DEPS_INSTALL_RUNTIME_DIR/jdk-install/bin:$PATH
export JAVA_HOME=$DEPS_INSTALL_RUNTIME_DIR/jdk-install

###############
# Google Test #
###############

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://github.com/google/googletest/archive/release-1.10.0.tar.gz
tar -xf release-1.10.0.tar.gz
mkdir gtest-build
cd gtest-build
cmake ../googletest-release-1.10.0 -DCMAKE_INSTALL_PREFIX=$DEPS_INSTALL_BUILD_DIR/gtest-install
make install --quiet --jobs $(nproc)
rm -f $PACKAGES_DIR/release-1.10.0.tar.gz

#######
# npm #
#######

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://nodejs.org/dist/v16.20.2/node-v16.20.2-linux-x64.tar.xz
tar -xf node-v16.20.2-linux-x64.tar.xz -C $DEPS_INSTALL_RUNTIME_DIR
rm -f node-v16.20.2-linux-x64.tar.xz

cd $DEPS_INSTALL_RUNTIME_DIR
mv node-v16.20.2-linux-x64 node-install
export PATH=$DEPS_INSTALL_RUNTIME_DIR/node-install/bin:$PATH

############
# OpenLDAP #
############

cd $PACKAGES_DIR
wget --no-verbose --no-clobber http://mirror.eu.oneandone.net/software/openldap/openldap-release/openldap-2.5.6.tgz
tar -xf openldap-2.5.6.tgz

cd openldap-2.5.6
./configure --quiet --prefix=$DEPS_INSTALL_RUNTIME_DIR/openldap-install
make depend
make
make install

rm -f $PACKAGES_DIR/openldap-2.5.6.tgz

#######
# pip #
#######

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://bootstrap.pypa.io/get-pip.py
python3 get-pip.py
rm -f get-pip.py

#############
# ccdb-tool #
#############

cd $PACKAGES_DIR
wget --no-verbose --no-clobber https://github.com/gamesh411/ccdb-tool/archive/master.zip -O ccdb-tool.zip
unzip -q ccdb-tool.zip -d $BUILD_DIR
rm -f ccdb-tool.zip

cd $BUILD_DIR
mv ccdb-tool-master ccdb-tool
