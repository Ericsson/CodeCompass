#!/usr/bin/env bash
set -e

declare build_dir="/tmp/build"

function cleanup() {
    rm --recursive --force "${build_dir}" 
}

mkdir --parent "${build_dir}"
trap cleanup EXIT

declare running_ubuntu_codename="$(lsb_release --codename --short)"
declare source_dir=""
if [[ "${running_ubuntu_codename}" == "xenial" ]]; then
    source_dir=/usr/src/gtest
elif [[ "${running_ubuntu_codename}" == "bionic" ]]; then
    source_dir=/usr/src/googletest
else
    echo "Unsupported ubuntu release" 2>&1
    exit 1
fi

cp --recursive --target-directory="${build_dir}" "${source_dir}/"*
cd "${build_dir}"

if [[ "${running_ubuntu_codename}" == "xenial" ]]; then
    cmake "."
    make --jobs=$(nproc)
    cp --target-directory="/usr/lib" "libgtest.a" "libgtest_main.a"
else
    cmake "." -DCMAKE_INSTALL_PREFIX="/usr/lib"
    make --jobs=$(nproc) install
fi

