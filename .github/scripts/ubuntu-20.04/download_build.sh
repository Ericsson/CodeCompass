# Download installers for compiled dependencies

mkdir -p ${DOWNLOAD_PATH}

## Thrift 0.16

wget -O ${DOWNLOAD_PATH}/thrift-0.16.0.tar.gz "http://archive.apache.org/dist/thrift/0.16.0/thrift-0.16.0.tar.gz"

# Calculate hash of dependencies for Github Cache Action

dependencies_to_hash=("thrift-0.16.0.tar.gz")

concatenated_hashes=""
for file in "${dependencies_to_hash[@]}"; do
    file_hash=$(md5sum "${DOWNLOAD_PATH}/${file}" | awk '{print $1}')
    concatenated_hashes="${concatenated_hashes}${file_hash}"
done

hash_value=$(echo -n "$concatenated_hashes" | md5sum | awk '{print $1}')

## Save said hash

echo "compile-hash-key=${hash_value}" >> $GITHUB_OUTPUT