#!/usr/bin/env bash

set -e

function usage() {
    cat <<EOF
${0}
    Options:
        <Directory of CodeCompass source>
        <Output directory>
EOF
}

if [[ -z "${2}" ]]; then
    echo "Mandatory options is not specified." >&2
    usage >&2
    exit 1
fi

if [[ ! -z "${3}" ]]; then
    echo "Too many options." >&2
    usage >&2
    exit 2
fi

CODE_COMPASS_SRC_DIR="${1}"
CODE_COMPASS_OUTPUT_DIR="${2}"

CODE_COMPASS_BUILD_DIR="${CODE_COMPASS_OUTPUT_DIR}/build"
CODE_COMPASS_INSTALL_DIR="${CODE_COMPASS_OUTPUT_DIR}/install"
CODE_COMPASS_NPM_DIR="${CODE_COMPASS_OUTPUT_DIR}/npm"

mkdir -p "${CODE_COMPASS_BUILD_DIR}"
mkdir -p "${CODE_COMPASS_INSTALL_DIR}"
mkdir -p "${CODE_COMPASS_NPM_DIR}"

SCRIPT_DIR=$(readlink -ev "$(dirname "$(which "${0}")")")
source "${SCRIPT_DIR}/builder_config.sh"

export NPM_CONFIG_PREFIX="${CODE_COMPASS_NPM_DIR}"

cd "${CODE_COMPASS_BUILD_DIR}"
make -j $(nproc)
make install
