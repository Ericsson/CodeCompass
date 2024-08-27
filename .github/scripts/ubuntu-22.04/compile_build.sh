#!/bin/bash

# Compile dependencies that cannot be acquired via the official repositories

mkdir -p ${INSTALL_PATH}

## Compile build2

sh "${DOWNLOAD_PATH}/install_latest_build2.sh" ${INSTALL_PATH}/build2

## Compile odb, libodb, and its connectors

mkdir -p ${DOWNLOAD_PATH}/odb
cd ${DOWNLOAD_PATH}/odb
${INSTALL_PATH}/build2/bin/bpkg create --quiet --jobs $(nproc) cc \
  config.cxx=g++ \
  config.cc.coptions=-O3 \
  config.bin.rpath=${INSTALL_PATH}/odb/lib \
  config.install.root=${INSTALL_PATH}/odb

### Getting the source
${INSTALL_PATH}/build2/bin/bpkg add https://pkg.cppget.org/1/beta --trust-yes
${INSTALL_PATH}/build2/bin/bpkg fetch --trust-yes

### Building odb
${INSTALL_PATH}/build2/bin/bpkg build odb --yes
${INSTALL_PATH}/build2/bin/bpkg build libodb --yes
${INSTALL_PATH}/build2/bin/bpkg build libodb-sqlite --yes
${INSTALL_PATH}/build2/bin/bpkg build libodb-pgsql --yes
${INSTALL_PATH}/build2/bin/bpkg install --all --recursive