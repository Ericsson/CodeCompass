#!/usr/bin/env bash

set -e

function usage() {
    echo "${0}"
    echo "    <Directory of CodeCompass source will be placed to>"
    echo "    <URL where the CodeCompass source will be fetched from>"
}

if [[ -z "${2}" ]]; then
    echo "Too few option(s)." >&2
    usage
    exit 1
fi

if [[ ! -z "${3}" ]]; then
    echo "Too many options." >&2
    usage
    exit 2
fi

MOUNTED_SRC_DIR="${1}"
COMPASS_URL="${2}"
git clone "${COMPASS_URL}" "${MOUNTED_SRC_DIR}"

