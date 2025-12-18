#!/bin/bash

# Download installers for compiled dependencies

mkdir -p "${DOWNLOAD_PATH}"

## Thrift 0.16 + ODB beta

wget -O "${DOWNLOAD_PATH}/install_latest_build2.sh" "https://github.com/Ericsson/CodeCompass/raw/master/scripts/install_latest_build2.sh"
build2_version=$(sh "${DOWNLOAD_PATH}/install_latest_build2.sh" --version)
odb_signature=$(wget -qO- https://pkg.cppget.org/1/beta/signature.manifest)

# Calculate hash of dependencies for Github Cache Action

hash_value=$(echo -n "${build2_version}${odb_signature}" | md5sum | awk '{print $1}')

## Save said hash

### Restore action
echo "compile-hash-key=${hash_value}" >> "$GITHUB_OUTPUT"

### Save action
echo "CACHE_KEY=${hash_value}" >> "$GITHUB_ENV"