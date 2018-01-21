#!/usr/bin/env bash

function cleanup() {
    echo "Cleaning up Odb temporaries."
    if  [ -n "${ODB_BUILD_DIR}" ]; then
        rm -rf "${ODB_BUILD_DIR}"
    fi
}

trap cleanup EXIT

set -e

export CC=gcc
export CXX=g++

ODB_MIRROR="http://www.codesynthesis.com/download/odb"
ODB_MAJOR_VERSION="2.4"
ODB_FULL_VERSION="${ODB_MAJOR_VERSION}.0"
ODB_INSTALL_DIR="/opt/odb"
ODB_BUILD_DIR="/tmp/odb"

ODB_LIB_SRC_DIR="${ODB_BUILD_DIR}/odb_lib"
ODB_LIB_ARCHIVE_NAME="libodb-${ODB_FULL_VERSION}.tar.gz"
mkdir -p "${ODB_LIB_SRC_DIR}"
wget --no-verbose                                                              \
  "${ODB_MIRROR}/${ODB_MAJOR_VERSION}/${ODB_LIB_ARCHIVE_NAME}"                 \
  -O "${ODB_BUILD_DIR}/${ODB_LIB_ARCHIVE_NAME}"
tar -xzf "${ODB_BUILD_DIR}/${ODB_LIB_ARCHIVE_NAME}" -C "${ODB_LIB_SRC_DIR}"    \
  --strip-components=1
rm "${ODB_BUILD_DIR}/${ODB_LIB_ARCHIVE_NAME}"

pushd "${ODB_LIB_SRC_DIR}"
./configure "--prefix=${ODB_INSTALL_DIR}"
make -j $(nproc) install
popd

ODB_PGSQL_ARCHIVE_NAME="libodb-pgsql-${ODB_FULL_VERSION}.tar.gz"
ODB_PGSQL_SRC_DIR="${ODB_BUILD_DIR}/odb_psql"
mkdir -p "${ODB_PGSQL_SRC_DIR}"
wget --no-verbose                                                              \
  "${ODB_MIRROR}/${ODB_MAJOR_VERSION}/${ODB_PGSQL_ARCHIVE_NAME}"               \
  -O "${ODB_BUILD_DIR}/${ODB_PGSQL_ARCHIVE_NAME}"
tar -xzf "${ODB_BUILD_DIR}/${ODB_PGSQL_ARCHIVE_NAME}" -C "${ODB_PGSQL_SRC_DIR}"\
  --strip-components=1
rm "${ODB_BUILD_DIR}/${ODB_PGSQL_ARCHIVE_NAME}"

pushd "${ODB_PGSQL_SRC_DIR}"
./configure "--prefix=${ODB_INSTALL_DIR}" "--with-libodb=${ODB_LIB_SRC_DIR}"
make -j $(nproc) install
popd

ODB_SQLITE_ARCHIVE_NAME="libodb-sqlite-${ODB_FULL_VERSION}.tar.gz"
ODB_SQLITE_SRC_DIR="${ODB_BUILD_DIR}/odb_sqlite"
mkdir -p "${ODB_SQLITE_SRC_DIR}"
wget --no-verbose                                                              \
  "${ODB_MIRROR}/${ODB_MAJOR_VERSION}/${ODB_PGSQL_ARCHIVE_NAME}"               \
  -O "${ODB_BUILD_DIR}/${ODB_SQLITE_ARCHIVE_NAME}"
tar -xzf "${ODB_BUILD_DIR}/${ODB_SQLITE_ARCHIVE_NAME}"                         \
  -C "${ODB_SQLITE_SRC_DIR}" --strip-components=1
rm "${ODB_BUILD_DIR}/${ODB_SQLITE_ARCHIVE_NAME}"

pushd "${ODB_SQLITE_SRC_DIR}"
./configure "--prefix=${ODB_INSTALL_DIR}" "--with-libodb=${ODB_LIB_SRC_DIR}"
make -j $(nproc) install
popd


ODB_SRC_DIR="${ODB_BUILD_DIR}/odb"
ODB_ARCHIVE_NAME="odb-${ODB_FULL_VERSION}.tar.gz"
mkdir -p "${ODB_SRC_DIR}"
wget --no-verbose                                                              \
  "${ODB_MIRROR}/${ODB_MAJOR_VERSION}/${ODB_ARCHIVE_NAME}"                     \
  -O "${ODB_BUILD_DIR}/${ODB_ARCHIVE_NAME}"
tar -xzf "${ODB_BUILD_DIR}/${ODB_ARCHIVE_NAME}" -C "${ODB_SRC_DIR}"            \
  --strip-components=1
rm "${ODB_BUILD_DIR}/${ODB_ARCHIVE_NAME}"

pushd "${ODB_SRC_DIR}"
./configure "--prefix=${ODB_INSTALL_DIR}"
make -j $(nproc) install
popd

