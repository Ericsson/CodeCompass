#!/usr/bin/env bash

COMPASS_INSTALL_DIR="/opt/CodeCompass"

set -e

function usage() {
    echo "${0} <URL of CodeCompass source in a git repository>"
}

if [[ -z "${1}" ]]; then
    echo "URL of CodeCompass is not specified." >&2
    usage
    exit 1
fi

if [[ ! -z "${2}" ]]; then
    echo "Too many options." >&2
    usage
    exit 2
fi

COMPASS_URL="${1}"

scriptdir=$(readlink -ev "$(dirname "$(which "$0")")")

COMPASS_SRC_DIR="/tmp/Compass.main"
COMPASS_OUTPUT_DIR="/tmp/Compass.build"

mkdir -p "${COMPASS_SRC_DIR}"
mkdir -p "${COMPASS_OUTPUT_DIR}"

fetchcc.sh "${COMPASS_SRC_DIR}" "${COMPASS_URL}"
configurecc.sh "${COMPASS_SRC_DIR}" "${COMPASS_OUTPUT_DIR}" "Release"
buildcc.sh "${COMPASS_SRC_DIR}" "${COMPASS_OUTPUT_DIR}"

mv "${COMPASS_OUTPUT_DIR}/install" "${COMPASS_INSTALL_DIR}"
