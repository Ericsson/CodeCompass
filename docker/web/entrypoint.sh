#!/bin/bash

if [ "$(id -u)" == '0' ]; then
  # Change the owner of the workspace directory
  mkdir -p /workspace
  chown codecompass:codecompass /workspace

  # Execute this script again with codecompass user.
  exec gosu codecompass "$0" "$@"
fi

set -es
# Set default value for API_PREFIX if not provided
: "${API_PREFIX:=""}"

# Create web_config.env file with the API_PREFIX
cat > ../../webgui-new/.env.local <<EOF
API_PREFIX=/${API_PREFIX}
EOF

exec "$@"
