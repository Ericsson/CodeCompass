#!/bin/bash

set -Eeuo pipefail

#Usage information
function usage() {
    cat <<EOF
    This is a helper script to build CodeCompass's docker images.
    
${0} []
    -h  Print this usage information.
    -d Only build the CodeCompass:dev image.
    -R Remove and then rebuild both images.
${1} []
    -w Only build the CodeCopmass:webserver image. The tag name
    of the built developer image need to be passed.
EOF
}

#Checking dependecies
if [[ -z "$(which docker)" ]] ;then
    echo "Docker is not installed!"
    exit 1
fi

#Parsing arguments
declare opts
while getopts "hdw:R" option; do
    case ${option} in
        h)
            usage
            exit 0
            ;;
        d)
            opts="dev"
            ;;
        w)
            opts="web"
            declare cc_dev_image_name="${OPTARG}"
            ;;
        R)
            opts="rebuild"
            ;;
        *)
            usage >&2
            exit 1
            ;;
    esac
done

#Ensuring script location
declare -a script_path=$(readlink --canonicalize-existing --verbose "$(pwd)")
declare -a cc_root=$(git rev-parse --show-toplevel)
cd ${cc_root}/docker

#Determining build task
declare docker_command
if [[ -z "${opts:-}" ]]; then #Simply build both of the images
    echo "Full build"
    docker_command=("docker" "build" "-t" "codecompass:dev" "./dev" \
                   "&&" "docker" "build" "--rm" "codecompass:webserver" "./web")
elif [[ "${opts}" == "dev" ]]; then #Only build the developer image
    echo "Only building the CodeCompass developer image"
    docker_command=("docker" "build" "-t" "codecompass:dev" "./dev")
elif [[ "${opts}" == "web" ]]; then #Only build the webserver image
    echo "Only building the webserver's docker image"
    docker_command=("docker" "build" "--build-arg" "DEV_IMAGE=${cc_dev_image_name}" "-t" "codecompass:dev" "./web")
elif [[ "${opts}" == "rebuild" ]]; then #Remove and then rebuild the images
    echo "Rebuilding images"
    docker_command=("docker" "rmi" "CodeCompass" "-f" \
                   "docker" "build" "-t" "codecompass:dev" "./dev" \
                   "&&" "docker" "build" "codecompass:webserver" "./web")
fi

#Determining user permissions to run docker
if [[ "$(id -nG ${USER})" == *"docker"* ]]; then
    echo "Starting build process, as $(id -nu)"
    "${docker_command[@]}"
else
    echo "Starting build process as administrator"
    sudo "${docker_command[@]}"
fi
