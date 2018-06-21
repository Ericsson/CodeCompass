#!/usr/bin/env bash

set -e

function usage() {
cat << EOF
${0}
    Options:
        <Directory of CodeCompass source will be placed to>
        <URL where the CodeCompass source will be fetched from>
        <Branch name in the git repository>
EOF
}

if [[ -z "${3}" ]]; then
    echo "Too few option(s)." >&2
    usage >&2
    exit 1
fi

if [[ ! -z "${4}" ]]; then
    echo "Too many options." >&2
    usage >&2
    exit 2
fi

MOUNTED_SRC_DIR="${1}"
COMPASS_URL="${2}"
COMPASS_BRANCH="${3}"
git clone --branch "${COMPASS_BRANCH}" "${COMPASS_URL}" "${MOUNTED_SRC_DIR}"

