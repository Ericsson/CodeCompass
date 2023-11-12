# Post compilation configuration for building (environmental variables, library location settings etc..)

echo "${INSTALL_PATH}/thrift/bin" >> $GITHUB_PATH
echo "CMAKE_PREFIX_PATH=${INSTALL_PATH}/thrift:$CMAKE_PREFIX_PATH" >> $GITHUB_ENV