# Post compilation configuration for building (environmental variables, library location settings etc..)

echo "${INSTALL_PATH}/thrift/bin" >> $GITHUB_PATH
echo "CMAKE_PREFIX_PATH=${INSTALL_PATH}/thrift:$CMAKE_PREFIX_PATH" >> $GITHUB_ENV

# Clean up dependency sources and intermediate binaries to save space
rm -f ${DOWNLOAD_PATH}/thrift-0.16.0.tar.gz
rm -rf ${DOWNLOAD_PATH}/thrift-0.16.0/