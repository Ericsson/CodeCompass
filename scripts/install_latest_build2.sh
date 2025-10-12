#!/bin/bash

if [ $# -lt 1 ]; then
    echo "No installation directory was provided for build2!" 1>&2
    exit 1
fi

## download phase

install_dir=$1
toolchain_file="toolchain.sha256"
if [ "$1" = "--version" ]; then
    wget -q --no-verbose --no-clobber https://download.build2.org/toolchain.sha256 -O "${toolchain_file}"
else
    wget --no-verbose --no-clobber https://download.build2.org/toolchain.sha256 -O "${toolchain_file}"
fi


version_line=$(grep -m 1 '' "$toolchain_file")
version_number=$(echo "$version_line" | awk '{print $2}')

### Return with version string only without actually installing build2 if "--version" flag is specified.
if [ "$1" = "--version" ]; then
    echo "${version_number}"
    rm -f "${toolchain_file}"
    exit 0
fi

version_to_install=build2-install-${version_number}.sh
download_url=https://download.build2.org/${version_number}/${version_to_install}
wget --no-verbose --no-clobber "${download_url}" -O "${version_to_install}"

## sha256 check phase

line_with_version=$(grep "$version_to_install" "$toolchain_file")
checksum_for_version=$(echo "$line_with_version" | awk '{print $1}')

if echo "${checksum_for_version} ${version_to_install}" | sha256sum -c; then
    echo "Build2 installer for version ${version_number} has been downloaded!"
else
    echo "Expected checksum for build2 installer version ${version_number} doesn't match! Probably the file has been corrupted! Please install it manually!" 1>&2
    echo "Expected: ${checksum_for_version}" 1>&2
    rm "$version_to_install" "$toolchain_file"
    exit 1
fi

## install phase

sh ${version_to_install} --yes --trust yes --jobs $(nproc) "${install_dir}"

## cleanup phase

compressed_toolchain="build2-toolchain-${version_number}.tar.xz"

rm -f "${toolchain_file}" "${version_to_install}" "${compressed_toolchain}"

echo "Build2 version ${version_number} has been successfully installed!"
