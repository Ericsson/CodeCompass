# Post compilation configuration for runtime (environmental variables, library location settings etc..)

echo "CMAKE_PREFIX_PATH=${INSTALL_PATH}/thrift/lib:$CMAKE_PREFIX_PATH" >> $GITHUB_ENV