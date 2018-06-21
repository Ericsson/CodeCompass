#!/usr/bin/env bash

function cleanup() {
    echo "Cleaning up LLVM temporaries."
    if  [[ -n "${LLVM_BUILD_DIR}" ]]; then
        rm --recursive --force "${LLVM_BUILD_DIR}"
    fi
}

trap cleanup EXIT

set -e

function usage() {
    cat <<EOF
"${0} [-h] [-l]"
-h  Print this usage information. Optional."
-l  LLVM version. For example '6.0.0'." Optional. The default is '6.0.0'
EOF
}

LLVM_VERSION="6.0.0"
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
            usage >&2
            exit 1
            ;;
    esac
done

LLVM_INSTALL_DIR="/opt/llvm"
LLVM_BUILD_DIR="/tmp/llvm"
LLVM_MIRROR="http://releases.llvm.org"

mkdir --parents "${LLVM_BUILD_DIR}"
pushd "${LLVM_BUILD_DIR}"

LLVM_SRC_ARCHIVE_NAME="llvm-${LLVM_VERSION}.src.tar.xz"
LLVM_CFE_ARCHIVE_NAME="cfe-${LLVM_VERSION}.src.tar.xz"
LLVM_RT_ARCHIVE_NAME="compiler-rt-${LLVM_VERSION}.src.tar.xz"

wget --no-verbose "${LLVM_MIRROR}/${LLVM_VERSION}/${LLVM_SRC_ARCHIVE_NAME}" &
wget --no-verbose "${LLVM_MIRROR}/${LLVM_VERSION}/${LLVM_CFE_ARCHIVE_NAME}" &
wget --no-verbose "${LLVM_MIRROR}/${LLVM_VERSION}/${LLVM_RT_ARCHIVE_NAME}" &
wait

mkdir --parents "llvm"
tar --extract --xz --strip-components=1                                        \
    --file "${LLVM_SRC_ARCHIVE_NAME}"                                          \
    --directory "llvm"
rm "${LLVM_SRC_ARCHIVE_NAME}"

mkdir --parents "llvm/tools/clang"
tar --extract --xz --strip-components=1                                        \
    --file "${LLVM_CFE_ARCHIVE_NAME}"                                          \
    --directory "llvm/tools/clang"
rm "${LLVM_CFE_ARCHIVE_NAME}"

mkdir --parents "llvm/projects/compiler-rt"
tar --extract --xz --strip-components=1                                        \
    --file "compiler-rt-${LLVM_VERSION}.src.tar.xz"                            \
    --directory "llvm/projects/compiler-rt"
rm "${LLVM_RT_ARCHIVE_NAME}"

mkdir build
cd build
export REQUIRES_RTTI=1
cmake -G "Unix Makefiles" "-DLLVM_ENABLE_RTTI=ON" "-DCMAKE_BUILD_TYPE=Release"                              \
  "-DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR}" "../llvm"
make -j $(nproc) install
popd
