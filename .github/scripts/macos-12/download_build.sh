#!/bin/bash

# Download installers for compiled dependencies

mkdir -p "${DOWNLOAD_PATH}"

## Thrift 0.16

wget -O ${DOWNLOAD_PATH}/thrift-0.16.0.tar.gz "http://archive.apache.org/dist/thrift/0.16.0/thrift-0.16.0.tar.gz"

## ODB

wget -O "${DOWNLOAD_PATH}/install_latest_build2.sh" "https://github.com/Ericsson/CodeCompass/raw/master/scripts/install_latest_build2.sh"
build2_version=$(sh "${DOWNLOAD_PATH}/install_latest_build2.sh" --version)
odb_signature=$(wget -qO- https://pkg.cppget.org/1/beta/signature.manifest)

# Calculate hash of dependencies for Github Cache Action

dependencies_to_hash=("thrift-0.16.0.tar.gz")

concatenated_hashes=""
for file in "${dependencies_to_hash[@]}"; do
    file_hash=$(md5sum "${DOWNLOAD_PATH}/${file}" | awk '{print $1}')
    concatenated_hashes="${concatenated_hashes}${file_hash}"
done
concatenated_hashes="${concatenated_hashes}${build2_version}${odb_signature}"

hash_value=$(echo -n "$concatenated_hashes" | md5sum | awk '{print $1}')

## Save said hash

### Restore action
echo "macos-12-compile-hash-key=${hash_value}" >> "$GITHUB_OUTPUT"

### Save action
echo "CACHE_KEY=${hash_value}" >> "$GITHUB_ENV"