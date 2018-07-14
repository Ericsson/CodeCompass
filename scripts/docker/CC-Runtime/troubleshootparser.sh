#!/usr/bin/env bash

set -e

function call_docker() {
    local command=("${@}")

    if [[ "$(id --name --groups)" == *"docker"* ]]                             \
       ||                                                                      \
       [[ ! -z ${DOCKER_HOST} ]]; then
        "${command[@]}"
    else
        sudo "${command[@]}"
    fi
}

script_dir=$(readlink --canonicalize-existing --verbose                        \
    "$(dirname "$(which "${0}")")")

docker_command="$(which docker)"
attach_to_parser_command=("${docker_command}" "attach" "cc_xercesparser")

# Attach to a running parser container
call_docker "${attach_to_parser_command[@]}"
