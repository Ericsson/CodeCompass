# Download installers for compiled dependencies

mkdir -p ${DOWNLOAD_PATH}

## Thrift 0.16

wget -O ${DOWNLOAD_PATH}/thrift-0.16.0.tar.gz "http://www.apache.org/dyn/mirrors/mirrors.cgi?action=download&filename=thrift/0.16.0/thrift-0.16.0.tar.gz"

# Calculate hash of dependencies for Github Cache Action

dependencies_to_hash=("thrift-0.16.0.tar.gz")

concatenated_data=""
for file in "${dependencies_to_hash[@]}"; do
    concatenated_data="${concatenated_data}$(echo -n "$file"; cat "${DOWNLOAD_PATH}/${file}")"
done

hash_value=$(echo -n "$concatenated_data" | md5sum | awk '{print $1}')

## Save said hash

echo "compile-hash-key=${hash_value}" >> $GITHUB_OUTPUT