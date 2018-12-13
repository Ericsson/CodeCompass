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

compose_command="$(which docker-compose)"
yaml_file="${script_dir}/docker-compose.yaml"

cd "${script_dir}"

# Start containers.
# TODO: Services depends on each other. Should be synchronized.

start_dbadmin_command=("${compose_command}" "--file" "${yaml_file}" "up"       \
    "dbadmin")
start_parser_command=("cd" "${script_dir}" "&&" "${compose_command}"           \
    "--file" "${yaml_file}" "up" "xercesparser")

echo "To start dbadmin use the following command:"
echo "${start_dbadmin_command[*]}"

echo "Run parsing of xerces use the following command:"
echo "${start_parser_command[*]}"

start_webserver_command=("${compose_command}" "--file" "${yaml_file}" "up"     \
  "webserver")

echo "To start webserver use the following command:"
echo "${start_webserver_command[*]}"

start_backend_command=("${compose_command}" "--file" "${yaml_file}" "up" "db")
call_docker "${start_backend_command[@]}"
