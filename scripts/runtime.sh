#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

export ENV_ROOT="${DIR}"
export PATH="$ENV_ROOT/bin:$PATH"
export LD_LIBRARY_PATH="$ENV_ROOT/lib:$CC_INSTALL_ROOT/lib:$ENV_ROOT/lib64:$CC_INSTALL_ROOT/lib64:$LD_LIBRARY_PATH"
export SASL_PATH="$ENV_ROOT/lib/sasl2"
export MAGIC="$ENV_ROOT/share/misc/magic.mgc"
