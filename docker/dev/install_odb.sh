#!/bin/sh

opt=${1:-"all"}

wget https://raw.githubusercontent.com/Ericsson/CodeCompass/master/scripts/install_latest_build2.sh
sh install_latest_build2.sh "/build2_install"
export PATH=/build2_install/bin:$PATH
# Configuring the build
mkdir /odb_build
cd /odb_build
bpkg create --quiet --jobs $(nproc) cc \
  config.cxx=g++ \
  config.cc.coptions=-O3 \
  config.bin.rpath=/usr/local/lib \
  config.install.root=/usr/local
# Getting the source
bpkg add https://pkg.cppget.org/1/beta --trust-yes
bpkg fetch --trust-yes
# Building ODB
BUILD_LIST="libodb"
case $opt in
  "sqlite")
    BUILD_LIST="$BUILD_LIST libodb-sqlite"
  ;;
  "pgsql")
    BUILD_LIST="$BUILD_LIST libodb-pgsql"
  ;;
  *)
    BUILD_LIST="$BUILD_LIST odb libodb-sqlite libodb-pgsql"
  ;;
esac
for pack in "$BUILD_LIST"; do
  bpkg build $pack --yes
done
# Install ODB (to /usr/local)
INSTALL_LIST="$BUILD_LIST libstudxml libcutl"
for pack in "$INSTALL_LIST"; do
  bpkg install $pack
done
# Clean up
cd /
sh install_latest_build2.sh --uninstall
rm -rf /odb_build install_latest_build2.sh build2-toolchain-*.tar.gz