#!/bin/bash

set -Eeuo pipefail

if [ "$(id -u)" == '0' ]; then
  # Change the owner of the workspace directory
  mkdir -p /workspace
  chown codecompass:codecompass /workspace

  # Execute this script again with codecompass user.
  exec gosu codecompass "$0" "$@"
fi

exec "$@"
