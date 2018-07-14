#!/usr/bin/env bash

set -e

function usage() {
    cat <<EOF
${0}
    Options:
        <Directory of CodeCompass source>
        <Output directory>
        <Build type>
        <Database type>
EOF
}

if [[ -z "${4}" ]]; then
    echo "Mandatory options is not specified." >&2
    usage >&2
    exit 1
fi

if [[ ! -z "${5}" ]]; then
    echo "Too many options." >&2
    usage >&2
    exit 2
fi

CODE_COMPASS_SRC_DIR="${1}"
CODE_COMPASS_OUTPUT_DIR="${2}"
CODE_COMPASS_BUILD_TYPE="${3}"
CODE_COMPASS_DATABASE_TYPE="${4}"

CODE_COMPASS_BUILD_DIR="${CODE_COMPASS_OUTPUT_DIR}/build"
CODE_COMPASS_INSTALL_DIR="${CODE_COMPASS_OUTPUT_DIR}/install"
CODE_COMPASS_NPM_DIR="${CODE_COMPASS_OUTPUT_DIR}/npm"

mkdir "${CODE_COMPASS_BUILD_DIR}"
mkdir "${CODE_COMPASS_INSTALL_DIR}"
mkdir "${CODE_COMPASS_NPM_DIR}"

SCRIPT_DIR=$(readlink --canonicalize-existing --verbose                        \
    "$(dirname "$(which "${0}")")")
source "${SCRIPT_DIR}/builder_config.sh"

export NPM_CONFIG_PREFIX="${CODE_COMPASS_NPM_DIR}"
# Workaround. TODO: It should be handled in cmake files.
export JAVA_TOOL_OPTIONS="-Dfile.encoding=UTF8"
cd "${CODE_COMPASS_BUILD_DIR}"

cmake "${CODE_COMPASS_SRC_DIR}"                                                \
  "-DCMAKE_INSTALL_PREFIX=${CODE_COMPASS_INSTALL_DIR}"                         \
  "-DDATABASE=${CODE_COMPASS_DATABASE_TYPE}"                                   \
  "-DCMAKE_BUILD_TYPE=${CODE_COMPASS_BUILD_TYPE}"

# TODO: Later the CodeCompass should be compiled with clang.
#  "-DCMAKE_C_COMPILER_ID=Clang" \
#  "-DCMAKE_CXX_COMPILER_ID=Clang" \
#  "-DCMAKE_C_COMPILER=clang" \
#  "-DCMAKE_CXX_COMPILER=clang++"
