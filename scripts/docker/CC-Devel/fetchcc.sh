#!/usr/bin/env bash

set -e

function usage() {
cat << EOF
${0} [-h]
${0} [-s <source directory>] [-u <repository URL>] [-b]
  -h  Print this usage information. Optional.
  -s  Directory of CodeCompass source. If not specified then
      CC_SOURCE environment variable will be used. Any of them
      is mandatory.
  -u  URL of repository of CodeCompass. If not specified then CC_URL environment
      variable will be used. If not specified then the the main repository
       (https://github.com/Ericsson/CodeCompass) will be used.
  -b  Branch of CodeCompass in the repository. Optional. If not specified then
      the the master branch will be used.
EOF
}

main_repo_url="https://github.com/Ericsson/CodeCompass"

cc_source_dir="${CC_SOURCE}"
cc_branch="master"
cc_output_dir="${CC_BUILD}"
cc_build_type="${CC_BUILD_TYPE}"
cc_url="${CC_URL}"
if [[ -z ${cc_url} ]]; then
    cc_url="${main_repo_url}"
fi
while getopts "hs:u:b:" option; do
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
        b)
            cc_branch="${OPTARG}"
            ;;
        *)
            usage >&2
            exit 1
            ;;
    esac
done

if [[ -z "${cc_source_dir}" ]]; then
    echo "Target directory of CodeCompass source was not specified." >&2
    usage >&2
    exit 2
fi

developer_id="$(id --user)"
developer_group="$(id --group)"

if [[ "${developer_id}" -eq 0 ]] || [[ "${developer_group}" -eq 0 ]]; then
    echo "'${0}' should not run as root." >&2
    exit 2
fi

mkdir --parents ${cc_source_dir}
cc_source_dir=$(readlink --canonicalize-existing --verbose "${cc_source_dir}")
cc_source_mounted="/mnt/cc_source"
docker_command=("docker" "run" "--rm"                                          \
  "--user=${developer_id}:${developer_group}"                                  \
  "--mount=type=bind,source=${cc_source_dir},target=${cc_source_mounted}"      \
  "compass-devel" "/usr/local/bin/fetchcompass.sh" "${cc_source_mounted}"      \
  "${cc_url}" "${cc_branch}")

if [[ "$(id -nG ${USER})" == *"docker"* ]] || [[ ! -z ${DOCKER_HOST} ]]; then
    "${docker_command[@]}"
else
    sudo "${docker_command[@]}"
fi
