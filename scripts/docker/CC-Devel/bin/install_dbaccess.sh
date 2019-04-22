#!/usr/bin/env bash

function cleanup() {
    echo "Cleaning up Odb temporaries."
    if [[ -n "${ODB_BUILD_DIR}" ]]; then
        rm --recursive --force "${ODB_BUILD_DIR}"
    fi
}

trap cleanup EXIT

set -e

if [[ "${1}" == "18.04" ]]; then
    exit 0 
fi

export CC="gcc-5"
export CXX="g++-5"

ODB_MIRROR="http://www.codesynthesis.com/download/odb"
ODB_MAJOR_VERSION="2.4"
ODB_FULL_VERSION="${ODB_MAJOR_VERSION}.0"
ODB_INSTALL_DIR="/opt/odb"
ODB_BUILD_DIR="/tmp/odb"

ODB_LIB_SRC_DIR="${ODB_BUILD_DIR}/odb_lib"
ODB_LIB_ARCHIVE_NAME="libodb-${ODB_FULL_VERSION}.tar.gz"
mkdir --parents "${ODB_LIB_SRC_DIR}"
wget --no-verbose                                                              \
  "${ODB_MIRROR}/${ODB_MAJOR_VERSION}/${ODB_LIB_ARCHIVE_NAME}"                 \
  --output-document="${ODB_BUILD_DIR}/${ODB_LIB_ARCHIVE_NAME}"
tar --extract --gunzip --file="${ODB_BUILD_DIR}/${ODB_LIB_ARCHIVE_NAME}"       \
  --directory="${ODB_LIB_SRC_DIR}" --strip-components=1
rm "${ODB_BUILD_DIR}/${ODB_LIB_ARCHIVE_NAME}"

pushd "${ODB_LIB_SRC_DIR}"
./configure "--prefix=${ODB_INSTALL_DIR}"
make --jobs=$(nproc) install
popd

ODB_PGSQL_ARCHIVE_NAME="libodb-pgsql-${ODB_FULL_VERSION}.tar.gz"
ODB_PGSQL_SRC_DIR="${ODB_BUILD_DIR}/odb_psql"
mkdir --parents "${ODB_PGSQL_SRC_DIR}"
wget --no-verbose                                                              \
  "${ODB_MIRROR}/${ODB_MAJOR_VERSION}/${ODB_PGSQL_ARCHIVE_NAME}"               \
  --output-document="${ODB_BUILD_DIR}/${ODB_PGSQL_ARCHIVE_NAME}"
tar --extract --gunzip --file="${ODB_BUILD_DIR}/${ODB_PGSQL_ARCHIVE_NAME}"     \
  --directory="${ODB_PGSQL_SRC_DIR}" --strip-components=1
rm "${ODB_BUILD_DIR}/${ODB_PGSQL_ARCHIVE_NAME}"

pushd "${ODB_PGSQL_SRC_DIR}"
./configure "--prefix=${ODB_INSTALL_DIR}" "--with-libodb=${ODB_LIB_SRC_DIR}"
make --jobs=$(nproc) install
popd

ODB_SQLITE_ARCHIVE_NAME="libodb-sqlite-${ODB_FULL_VERSION}.tar.gz"
ODB_SQLITE_SRC_DIR="${ODB_BUILD_DIR}/odb_sqlite"
mkdir --parents "${ODB_SQLITE_SRC_DIR}"
wget --no-verbose                                                              \
  "${ODB_MIRROR}/${ODB_MAJOR_VERSION}/${ODB_SQLITE_ARCHIVE_NAME}"              \
  --output-document="${ODB_BUILD_DIR}/${ODB_SQLITE_ARCHIVE_NAME}"
tar --extract --gunzip --file="${ODB_BUILD_DIR}/${ODB_SQLITE_ARCHIVE_NAME}"    \
  --directory="${ODB_SQLITE_SRC_DIR}" --strip-components=1
rm "${ODB_BUILD_DIR}/${ODB_SQLITE_ARCHIVE_NAME}"

pushd "${ODB_SQLITE_SRC_DIR}"
./configure "--prefix=${ODB_INSTALL_DIR}" "--with-libodb=${ODB_LIB_SRC_DIR}"
make --jobs=$(nproc) install
popd

ODB_SRC_DIR="${ODB_BUILD_DIR}/odb"
ODB_ARCHIVE_NAME="odb-${ODB_FULL_VERSION}.tar.gz"
mkdir --parents "${ODB_SRC_DIR}"
wget --no-verbose                                                              \
  "${ODB_MIRROR}/${ODB_MAJOR_VERSION}/${ODB_ARCHIVE_NAME}"                     \
  --output-document="${ODB_BUILD_DIR}/${ODB_ARCHIVE_NAME}"
tar --extract --gunzip --file="${ODB_BUILD_DIR}/${ODB_ARCHIVE_NAME}"           \
  --directory="${ODB_SRC_DIR}" --strip-components=1
rm "${ODB_BUILD_DIR}/${ODB_ARCHIVE_NAME}"

pushd "${ODB_SRC_DIR}"
./configure "--prefix=${ODB_INSTALL_DIR}"
make --jobs=$(nproc) install
popd
