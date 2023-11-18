#!/bin/bash

# Post compilation configuration for runtime (environmental variables, library location settings etc..)

echo "LD_LIBRARY_PATH=${INSTALL_PATH}/odb/lib:$LD_LIBRARY_PATH" >> $GITHUB_ENV