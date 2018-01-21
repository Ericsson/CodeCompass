#!/usr/bin/env bash

function cleanup() {
    echo "Cleaning up LLVM temporaries."
    if  [ -n "${LLVM_BUILD_DIR}" ]; then
        rm -rf "${LLVM_BUILD_DIR}"
    fi
}

trap cleanup EXIT

set -e

function usage() {
    echo "${0}"
    echo "  -h  Print this usage information. Optional."
    echo "  -l  LLVM version. Mandatory. For example '3.8.0'."
}

while getopts ":hl:" OPTION; do
    case ${OPTION} in
        h)
            usage
            exit 0
            ;;
        l)
            LLVM_VERSION="${OPTARG}"
            ;;
        *)
            usage
            exit 1
            ;;
    esac
done

if [[ -z "${LLVM_VERSION}" ]]; then
    echo "LLVM version should be defined." >&2
    usage
    exit 1
fi

LLVM_INSTALL_DIR="/opt/llvm"
LLVM_BUILD_DIR="/tmp/llvm"
LLVM_MIRROR="http://releases.llvm.org"

mkdir -p "${LLVM_BUILD_DIR}"
pushd "${LLVM_BUILD_DIR}"

LLVM_SRC_ARCHIVE_NAME="llvm-${LLVM_VERSION}.src.tar.xz"
LLVM_CFE_ARCHIVE_NAME="cfe-${LLVM_VERSION}.src.tar.xz"
LLVM_RT_ARCHIVE_NAME="compiler-rt-${LLVM_VERSION}.src.tar.xz"

wget --no-verbose "${LLVM_MIRROR}/${LLVM_VERSION}/${LLVM_SRC_ARCHIVE_NAME}" &
wget --no-verbose "${LLVM_MIRROR}/${LLVM_VERSION}/${LLVM_CFE_ARCHIVE_NAME}" &
wget --no-verbose "${LLVM_MIRROR}/${LLVM_VERSION}/${LLVM_RT_ARCHIVE_NAME}" &
wait

mkdir -p llvm
tar -xf "${LLVM_SRC_ARCHIVE_NAME}" -C "llvm" --strip-components=1
rm "${LLVM_SRC_ARCHIVE_NAME}"
mkdir -p "llvm/tools/clang"
tar -xf "${LLVM_CFE_ARCHIVE_NAME}" -C "llvm/tools/clang" --strip-components=1
rm "${LLVM_CFE_ARCHIVE_NAME}"
mkdir -p "llvm/projects/compiler-rt"
tar -xf "compiler-rt-${LLVM_VERSION}.src.tar.xz"                               \
  -C "llvm/projects/compiler-rt"                                               \
  --strip-components=1
rm "compiler-rt-${LLVM_VERSION}.src.tar.xz"

mkdir build
cd build
export REQUIRES_RTTI=1
cmake -G "Unix Makefiles" "-DLLVM_ENABLE_RTTI=ON"                              \
  "-DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR}" "../llvm"
make -j $(nproc) install
popd

