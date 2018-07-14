#!/usr/bin/env bash

COMPASS_INSTALL_DIR="/opt/CodeCompass"

set -e

function usage() {
cat << EOF
${0}
    Options:
        <URL of CodeCompass source in a git repository>
        <Branch name in the git repository>
EOF
}

if [[ -z "${2}" ]]; then
    echo "Too few options specified." >&2
    usage
    exit 1
fi

if [[ ! -z "${3}" ]]; then
    echo "Too many options specified." >&2
    usage
    exit 2
fi

COMPASS_URL="${1}"
COMPASS_BRANCH="${2}"

COMPASS_SRC_DIR="/tmp/Compass.main"
COMPASS_OUTPUT_DIR="/tmp/Compass.build"

mkdir --parents "${COMPASS_SRC_DIR}"
mkdir --parents "${COMPASS_OUTPUT_DIR}"

fetchcompass.sh "${COMPASS_SRC_DIR}" "${COMPASS_URL}" "${COMPASS_BRANCH}"
configurecompass.sh "${COMPASS_SRC_DIR}" "${COMPASS_OUTPUT_DIR}" "Release"     \
    "pgsql"
buildcompass.sh "${COMPASS_SRC_DIR}" "${COMPASS_OUTPUT_DIR}"

mv "${COMPASS_OUTPUT_DIR}/install" "${COMPASS_INSTALL_DIR}"
