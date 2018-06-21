#!/usr/bin/env bash

set -e

function usage() {
    cat <<EOF
${0} [-h]
${0} [-s <source directory>] [-o <output directory>] [-t]
  -h  Print this usage information. Optional.
  -s  Directory of CodeCompass source. If not specified this option
      CC_SOURCE environment variable will be used. If any of them not specified,
      this script uses the root directory of this git repository as Compass
      source.
  -o  Directory of generated output artifacts. If not specified then
      CC_BUILD environment variable will be used. Any of them
      is mandatory.
  -t  Build type. Optional.
      It can be: Debug; Release; RelWithDebInfo; MinSizeRel
      If not specified then CC_BUILD_TYPE environment variable will
      be used. Any of them is mandatory.
EOF
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
            usage >&2
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
    script_dir=$(readlink -ev "$(dirname "$(which "${0}")")")
    cc_source_dir=$(
        set +e
        cd ${script_dir}
        git rev-parse --show-toplevel
    )

    if [[ ! $? ]]; then
        echo "CodeCompass source directory should be defined." >&2
        usage >&2
        exit 3
    fi
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
