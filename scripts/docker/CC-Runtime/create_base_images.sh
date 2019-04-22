#!/usr/bin/env bash

set -e

function usage() {
cat << EOF
${0} [-h]
${0} [-u <repository URL>] [-b <branch name>]
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
while getopts "b:hu:" option; do
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
            usage 2>&1
            exit 1
            ;;
    esac
done

#TODO Check unnecessary positional parameters.

script_dir=$(readlink --canonicalize-existing --verbose                        \
    "$(dirname "$(which "${0}")")")

docker_command="$(which docker)"

inspect_command=("${docker_command}" "inspect" "compass-devel")
ubuntu_version=$(call_docker "${inspect_command[@]}"                           \
                 | jq --raw-output                                             \
                 '.[].ContainerConfig.Labels."com.ericsson.codecompass.ubuntu.version"')

# Create an ubuntu container with installed CodeCompass software.
build_command=("${docker_command}" "build"                                     \
    "--build-arg" "UBUNTU_VERSION=${ubuntu_version}"                           \
    "--build-arg" "compass_source_url=${cc_url}"                               \
    "--build-arg" "compass_branch=${cc_branch}"                                \
    "--tag=codecompass" "${script_dir}/codecompass")
call_docker "${build_command[@]}"

# Create image as base image of CC parsers.
build_command=("${docker_command}" "build" "--tag=ccparser"                    \
    "${script_dir}/ccparser")
call_docker "${build_command[@]}"

# Create ".env" file for docker-compose command.
# Compilation process generate files in the filesystem of hosts. The
# docker-compose and yaml file supports that parser container run as developer
# of the host. In that case the host filesystem will not be polluted with files
# with root permissions.
yaml_environment_setter_file="${script_dir}/.env"
cat << EOF > "${yaml_environment_setter_file}"
user_id=$(id --user)
group_id=$(id --group)
EOF

compose_command="$(which docker-compose)"
yaml_file="${script_dir}/docker-compose.yaml"

cd "${script_dir}"
# Create whole leaf images of the networked application.
pull_command=("${compose_command}" "--file" "${yaml_file}" "pull" "db"         \
    "dbadmin")
call_docker "${pull_command[@]}"

build_command=("${compose_command}" "--file" "${yaml_file}" "build")
call_docker "${build_command[@]}"
