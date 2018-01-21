#!/usr/bin/env bash

set -e

function usage() {
    echo "${0}"
    echo "  -h  Print this usage information. Optional."
    echo "  -s  Directory of CodeCompass source. If not specified then"
    echo "      CC_SOURCE environment variable will be used. Any of them"
    echo "      is mandatory."
    echo "  -o  Directory of generated output artifacts. If not specified then"
    echo "      CC_BUILD environment variable will be used. Any of them"
    echo "      is mandatory."
    echo "  -t  Build type. Optional."
    echo "      It can be: Debug; Release; RelWithDebInfo; MinSizeRel"
    echo "      If not specified then CC_BUILD_TYPE environment variable will "
    echo "      be used. Any of them is mandatory."
}

cc_source_dir="${CC_SOURCE}"
cc_output_dir="${CC_BUILD}"
cc_build_type="${CC_BUILD_TYPE}"
while getopts ":hs:o:t:" option; do
    case ${option} in
        h)
            usage
            exit 0
            ;;
        s)
            cc_source_dir="${OPTARG}"
            ;;
        o)
            cc_output_dir="${OPTARG}"
            ;;
        t)
            cc_build_type="${OPTARG}"
            ;;
        *)
            usage
            exit 1
            ;;
    esac
done

if [[ "${cc_build_type}" != "Debug" ]] \
   && [[ "${cc_build_type}" != "Release" ]] \
   && [[ "${cc_build_type}" != "RelWithDebInfo" ]] \
   && [[ "${cc_build_type}" != "MinSizeRel" ]]; then
    echo "Unknown build type: '${cc_build_type}'." >&2
    usage
    exit 2
fi

if [[ -z "${cc_source_dir}" ]]; then
    echo "CodeCompass source directory should be defined." >&2
    usage
    exit 3
fi

if [[ -z "${cc_output_dir}" ]]; then
    echo "Output directory of build should be defined." >&2
    usage
    exit 4
fi

developer_id="$(id --user)"
developer_group="$(id --group)"

if [[ "$developer_id" -eq 0 ]] || [[ "$developer_group" -eq 0 ]]; then
    echo "'${0}' should not run as root." >&2
    exit 2
fi

mkdir -p ${cc_output_dir}
cc_source_mounted="/mnt/cc_source"
cc_output_mounted="/mnt/cc_output"

docker_command=("docker" "run" "--rm" \
  "--user=${developer_id}:${developer_group}" \
  "--mount" "type=bind,source=${cc_source_dir},target=${cc_source_mounted}" \
  "--mount" "type=bind,source=${cc_output_dir},target=${cc_output_mounted}" \
  "compass-devel" "/usr/local/bin/configurecc.sh" "${cc_source_mounted}" \
  "${cc_output_mounted}" "${cc_build_type}")
  
if [[ "$(id -nG ${USER})" == *"docker"* ]] || [[ ! -z ${DOCKER_HOST} ]]; then
    "${docker_command[@]}"
else
    sudo "${docker_command[@]}"
fi
