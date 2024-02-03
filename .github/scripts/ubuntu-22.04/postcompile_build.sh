#!/bin/bash

# Post compilation configuration for building (environmental variables, library location settings etc..)

echo "${INSTALL_PATH}/odb/bin" >> $GITHUB_PATH
echo "CMAKE_PREFIX_PATH=${INSTALL_PATH}/odb:$CMAKE_PREFIX_PATH" >> $GITHUB_ENV