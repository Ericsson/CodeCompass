#!/usr/bin/env bash

set -euxo pipefail

trap cleanup EXIT

declare ODB_INSTALL_DIR="/usr/local"
declare SUCCESS="false"
declare REMOVE_BUILDTWO="true"
declare BUILDTWO_BUILD_DIR
declare BUILDTWO_INSTALL_DIR
declare ODB_BUILD_DIR
BUILDTWO_BUILD_DIR="$(mktemp --directory /tmp/build2_build_XXXXXX)"
BUILDTWO_INSTALL_DIR="$(mktemp --directory /tmp/build2_install_XXXXXX)"
ODB_BUILD_DIR="$(mktemp --directory /tmp/odb_build_XXXXXX)"

function cleanup() {
    echo "Cleaning up temporaries."
    
    if [[ -d "${BUILDTWO_BUILD_DIR}" ]]; then
        rm --recursive --force "${BUILDTWO_BUILD_DIR}"
    fi
    
    if [[ -d "${ODB_BUILD_DIR}" ]]; then
        rm --recursive --force "${ODB_BUILD_DIR}"
    fi
    
    if [[ "${SUCCESS}" != "true" ]]; then
        echo "Cleaning up after failed installation."
        if [[ -d "${ODB_INSTALL_DIR}" ]]; then
            rm --recursive --force "${ODB_INSTALL_DIR}"
        fi
    
        if [[ -d "${BUILDTWO_INSTALL_DIR}" ]]; then
           rm --recursive --force "${BUILDTWO_INSTALL_DIR}"
        fi
    fi
    
    if [[ "${REMOVE_BUILDTWO}" != "false" ]]; then
       echo "Removing the build2 toochain"
       if [[ -d "${BUILDTWO_INSTALL_DIR}" ]]; then
           rm --recursive --force "${BUILDTWO_INSTALL_DIR}"
       fi
    fi
}

function usage() {
    cat <<EOF
This is a convenience script that installs the build2 build system and
uses that to build and install the odb compiler, runtime library and also
the slite and pgsql support library.

${0} [-h] [-d] <odb_install_path> 
  -h  Print this usage information, and quit.
  -d  Install directory of odb. Optional. /usr/local is the deafault.
  -k  Keep the build2 toolchain after the successful installation of odb,
      on the given path.
EOF
}

while getopts "hd:k:" OPTION; do
    case ${OPTION} in
        h)
            usage
            exit 0
            ;;
        d)
            ODB_INSTALL_DIR="${OPTARG}"
            ;;
        k)
            REMOVE_BUILDTWO="false"
            BUILDTWO_INSTALL_DIR="${OPTARG}"
            ;;
        *)
            usage >&2
            exit 1
            ;;
    esac
done


# Build2 toolchain
pushd "${BUILDTWO_BUILD_DIR}"
curl -sSfO https://download.build2.org/0.12.0/build2-install-0.12.0.sh
sh build2-install-0.12.0.sh --yes --trust yes "${BUILDTWO_INSTALL_DIR}"
popd


export PATH="${BUILDTWO_INSTALL_DIR}/bin:$PATH"

# Configuring the build

pushd "${ODB_BUILD_DIR}"
bpkg create --quiet --jobs "$(nproc)" cc         \
  config.cxx=g++                                 \
  config.cc.coptions=-O3                         \
  config.bin.rpath="${ODB_INSTALL_DIR}/lib"      \
  config.install.root="${ODB_INSTALL_DIR}"       \
# Getting the source
bpkg add https://pkg.cppget.org/1/beta --trust-yes
bpkg fetch --trust-yes
# Building odb 
bpkg build odb --yes
bpkg build libodb --yes
bpkg build libodb-sqlite --yes
bpkg build libodb-pgsql --yes
bpkg install --all --recursive
popd

SUCCESS="true"

