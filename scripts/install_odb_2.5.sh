#!/usr/bin/env bash

set -euxo pipefail

trap cleanup EXIT

declare ODB_INSTALL_DIR="/usr/local"
declare ODB_BUILD_DIR="/tmp/odb"

function cleanup() {
    echo "Cleaning up Odb temporaries."
    if [[ -n "${ODB_BUILD_DIR}" ]]; then
        rm --recursive --force "${ODB_BUILD_DIR}"
    fi
}

function usage() {
    cat <<EOF
This is a convenience script that installs the build2 build system and
uses that to build and install the odb compiler, runtime library and also
the slite and pgsql support library.

${0} [-h] [-d] <odb_install_path> 
  -h  Print this usage information, and quit.
  -d  Install directory of odb. Optional. /opt/odb is the deafault.
EOF
}

while getopts "hd:" OPTION; do
    case ${OPTION} in
        h)
            usage
            exit 0
            ;;
        d)
            ODB_INSTALL_DIR="${OPTARG}"
            ;;
        *)
            usage >&2
            exit 1
            ;;
    esac
done


#Build2 toolchain
mkdir /tmp/build2src
pushd "/tmp/build2src"
curl -sSfO https://download.build2.org/0.12.0/build2-install-0.12.0.sh
sh build2-install-0.12.0.sh --yes --trust yes
popd


#Building the odb compiler
mkdir /tmp/odb
pushd "/tmp/odb"
bpkg create --quiet --jobs $(nproc) cc \
  config.cxx=g++                       \
  config.cc.coptions=-O3               \
  config.bin.rpath=/usr/local/lib      \
  config.install.root=/usr/local       \

bpkg add https://pkg.cppget.org/1/beta --trust-yes
bpkg fetch --trust-yes
bpkg build odb --yes
bpkg build libodb --yes
bpkg build libodb-sqlite --yes
bpkg build libodb-pgsql --yes
bpkg install --all --recursive
popd
