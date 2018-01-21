#!/usr/bin/env bash

set -e

function usage() {
    echo "${0}"
    echo "  -h  Print this usage information. Optional."
    echo "  -s  Directory of CodeCompass source. If not specified then"
    echo "      CC_SOURCE environment variable will be used. Any of them"
    echo "      is mandatory."
    echo "  -u  URL of repository of CodeCompass. If not specified then"
    echo "      CC_URL environment variable will be used. If none of them was"
    echo "      specified then the the main repository"
    echo "      (https://github.com/Ericsson/CodeCompass) will be used."
}

main_repo_url="https://github.com/Ericsson/CodeCompass"

cc_source_dir="${CC_SOURCE}"
cc_output_dir="${CC_BUILD}"
cc_build_type="${CC_BUILD_TYPE}"
cc_url="${CC_URL}"
if [[ -z ${cc_url} ]]; then
    cc_url="${main_repo_url}"
fi
while getopts ":hs:u:" option; do
    case ${option} in
        h)
            usage
            exit 0
            ;;
        s)
            cc_source_dir="${OPTARG}"
            ;;
        u)
            cc_url="${OPTARG}"
            ;;
        *)
            usage
            exit 1
            ;;
    esac
done

if [[ -z "${cc_source_dir}" ]]; then
    echo "Target directory of CodeCompass source was not specified." >&2
    usage
    exit 2
fi

developer_id="$(id --user)"
developer_group="$(id --group)"

if [[ "$developer_id" -eq 0 ]] || [[ "$developer_group" -eq 0 ]]; then
    echo "'${0}' should not run as root." >&2
    exit 2
fi

mkdir -p ${cc_source_dir}
cc_source_mounted="/mnt/cc_source"
docker_command=("docker" "run" "--rm"                                          \
  "--user=${developer_id}:${developer_group}"                                  \
  "--mount=type=bind,source=${cc_source_dir},target=${cc_source_mounted}"      \
  "compass-devel" "/usr/local/bin/fetchcc.sh" "${cc_source_mounted}"           \
  "${cc_url}")

if [[ "$(id -nG ${USER})" == *"docker"* ]] || [[ ! -z ${DOCKER_HOST} ]]; then
    "${docker_command[@]}"
else
    sudo "${docker_command[@]}"
fi
