#!/usr/bin/env bash

set -e

function usage() {
    cat <<EOF
${0} [-h]
${0} [-s <source directory>] [-o <output directory>] [u]

  -h  Print this usage information. Optional.
  -s  Directory of CodeCompass source. If not specified this option
      CC_SOURCE environment variable will be used. If any of them not specified,
      this script uses the root directory of this git repository as Compass
      source.
  -o  Directory of generated output artifacts. If not specified then
      CC_BUILD environment variable will be used. Any of them"
      is mandatory."
  -u  Run container as the same user as caller. Otherwise as root.
EOF
}

cc_source_dir="${CC_SOURCE}"
cc_output_dir="${CC_BUILD}"
run_as_root="true"
while getopts "hs:o:u" OPTION; do
    case ${OPTION} in
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
        u)
            run_as_root="false"

            ;;
        *)
            usage >&2
            exit 1
            ;;
    esac
done

if [[ -z "${cc_source_dir}" ]]; then
    script_dir=$(readlink --canonicalize-existing --verbose "$(dirname "$(which "${0}")")")
    cc_source_dir=$(
        set +e
        cd ${script_dir}
        git rev-parse --show-toplevel
    )

    if [[ ! $? ]]; then
        echo "CodeCompass source directory should be defined." >&2
        usage >&2
        exit 2
    fi
fi
cc_source_dir=$(readlink --canonicalize-existing --verbose "$cc_source_dir")

if [[ -z "${cc_output_dir}" ]]; then
    echo "Output directory of build should be defined." >&2
    usage >&2
    exit 3
fi
cc_output_dir=$(readlink --canonicalize-existing --verbose "${cc_output_dir}")

developer_id="$(id --user)"
developer_group="$(id --group)"

if [[ "${developer_id}" -eq 0 ]] || [[ "${developer_group}" -eq 0 ]]; then
    echo "'${0}' should not run as root." >&2
    exit 4
fi

if [[ ! -d "${cc_source_dir}" ]]; then
    echo "Source directory does not exist." >&2
    usage >&2
    exit 5
fi

if [[ ! -d "${cc_output_dir}" ]]; then
    echo "Output directory does not exist." >&2
    usage >&2
    exit 6
fi

cc_source_mounted="/mnt/cc_source"
cc_output_mounted="/mnt/cc_output"

docker_command=("docker" "run" "--rm" "--interactive" "--tty")

if [[ "${run_as_root}" == "false" ]]; then
    docker_command+=("--user=${developer_id}:${developer_group}")
fi

docker_command+=(
  "--mount" "type=bind,source=${cc_source_dir},target=${cc_source_mounted}"    \
  "--mount" "type=bind,source=${cc_output_dir},target=${cc_output_mounted}"    \
  "compass-devel" "/bin/bash")

if [[ "$(id -nG ${USER})" == *"docker"* ]] || [[ ! -z ${DOCKER_HOST} ]]; then
    "${docker_command[@]}"
else
    sudo "${docker_command[@]}"
fi
