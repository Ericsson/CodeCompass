#!/bin/bash

if [ "$(id --user)" == '0' ]; then
  # Change the owner of the workspace directory
  mkdir --parents /workspace
  chown codecompass:codecompass /workspace

  # Execute this script again with codecompass user.
  exec gosu codecompass "$0" "$@"
fi

exec "$@"
