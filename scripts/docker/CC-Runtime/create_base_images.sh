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

script_dir=$(readlink -ev "$(dirname "$(which "${0}")")")

docker_command="$(which docker)"

# Create an ubuntu 16.04 container with installed CodeCompass software.
build_command=("${docker_command}" "build" "--build-arg"                       \
  "compass_source_url=https://github.com/zomen2/CodeCompass" "--tag"           \
  "codecompass" "codecompass")
call_docker "${build_command[@]}"

# Create image as base image of CC parsers.
build_command=("${docker_command}" "build" "--tag" "ccparser" "ccparser")
call_docker "${build_command[@]}"

# Create ".env" file for docker-compose command.
cat << EOF > "${script_dir}/.env"
user_id=$(id --user)
group_id=$(id --group)
EOF

compose_command="$(which docker-compose)"
yaml_file="${script_dir}/docker-compose.yaml"

cd "${script_dir}"
# Create whole leaf images of the networked application.
pull_command=("${compose_command}" "-f" "${yaml_file}" "pull" "db" "dbadmin")     
call_docker "${pull_command[@]}"

build_command=("${compose_command}" "-f" "${yaml_file}" "build")
call_docker "${build_command[@]}"

