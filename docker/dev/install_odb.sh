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
  config.bin.rpath=/odb_install/lib \
  config.install.root=/odb_install
# Getting the source
bpkg add https://pkg.cppget.org/1/beta --trust-yes
bpkg fetch --trust-yes
# Building odb
bpkg build libodb --yes
case $opt in
  "sqlite")
    bpkg build libodb-sqlite --yes
  ;;
  "pgsql")
    bpkg build libodb-pgsql --yes
  ;;
  *)
    bpkg build odb --yes
    bpkg build libodb-sqlite --yes
    bpkg build libodb-pgsql --yes
  ;;
esac
bpkg install --all --recursive
# Copy to /usr
cp -rn /odb_install/* /usr
# Clean up
cd /
sh install_latest_build2.sh --uninstall
rm -rf /odb_build /odb_install install_latest_build2.sh build2-toolchain-*.tar.gz