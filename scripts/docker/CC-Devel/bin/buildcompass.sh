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

CODE_COMPASS_SRC_DIR=$(readlink --canonicalize-existing --verbose "${1}")
CODE_COMPASS_OUTPUT_DIR=$(readlink --canonicalize-existing --verbose "${2}")

CODE_COMPASS_BUILD_DIR="${CODE_COMPASS_OUTPUT_DIR}/build"
CODE_COMPASS_INSTALL_DIR="${CODE_COMPASS_OUTPUT_DIR}/install"

mkdir --parents "${CODE_COMPASS_BUILD_DIR}"
mkdir --parents "${CODE_COMPASS_INSTALL_DIR}"

SCRIPT_DIR=$(readlink --canonicalize-existing --verbose                        \
    "$(dirname "$(which "${0}")")")
source "${SCRIPT_DIR}/builder_config.sh"

cd "${CODE_COMPASS_BUILD_DIR}"
cmake --build . -- --jobs $(nproc)
make install
