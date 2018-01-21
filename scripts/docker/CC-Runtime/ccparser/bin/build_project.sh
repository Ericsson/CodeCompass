#!/usr/bin/env bash

# This script is a part of the toolset. It delegates the call of the build
# command to the project specific API.

# Parameters:
# 1 - Directory where the configuration files are placed.
#
# 2 - Directory where the project specific files are placed.
#

# Load project specific API implementation
scriptdir=$(readlink -e "$(dirname "$(which "$0")")")
source "${scriptdir}/project_specific.sh"

build_the_project "${scriptdir}" "${1}" "${2}"
exit $?

