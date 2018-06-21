#!/usr/bin/env bash

set -e

function usage() {
cat << EOF
${0} [-h]
${0} [-u <repository URL>] [-b]
  -h  Print this usage information. Optional.
  -u  URL of repository of CodeCompass. If not specified then CC_URL environment
      variable will be used. If not specified then the the main repository
       (https://github.com/Ericsson/CodeCompass) will be used.
  -b  Branch of CodeCompass in the repository. Optional. If not specified then
      the the master branch will be used.
EOF
}

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

cc_branch="master"
cc_url="https://github.com/Ericsson/CodeCompass"
while getopts ":hu:" option; do
    case ${option} in
        h)
            usage
            exit 0
            ;;
        u)
            cc_url="${OPTARG}"
            ;;
        b)
            cc_branch="${OPTARG}"
            ;;
        *)
            usage
            exit 1
            ;;
    esac
done

script_dir=$(readlink -ev "$(dirname "$(which "${0}")")")

docker_command="$(which docker)"

# Create an ubuntu 16.04 container with installed CodeCompass software.
build_command=("${docker_command}" "build"                                     \
    "--build-arg" "compass_source_url=${cc_url}"                               \
    "--build-arg" "compass_branch=${cc_branch}"                                \
    "--tag" "codecompass" "codecompass")
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

