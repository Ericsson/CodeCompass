#!/usr/bin/env bash

# Parameters. All of them mandatory.
#
# ${1}   Host [name | IP] of the postgres database instance.
#
# ${2}   Port number of the database service listening on.
#
# ${3}   Database user name. DB user name that access to the postgres database
#        instance.
#
# ${4}   Database user's password.
#
# ${5}   Port number where the CodeCompass_webserver listening on.
#
# ${6}   Workspace directory of the CodeCompass_webserver.

DB_HOST="${1}"
DB_PORT="${2}"
DB_USER="${3}"
DB_PASS="${4}"
CC_PORT="${5}"
SHARED_WORKSPACE_DIR="${6}"

# Project name (database=...) must not in the DB connection string.
DB_CONNECTION_STRING="\
pgsql:host=${DB_HOST};\
port=${DB_PORT};\
user=${DB_USER};\
password=${DB_PASS}\
"

# Set environment variables to run CodeCompass
export LD_LIBRARY_PATH="/opt/thrift/lib:\
/opt/odb/lib:\
/opt/CodeCompass/lib:\
/opt/CodeCompass/lib/serviceplugin"
export PATH="/opt/CodeCompass/bin:${PATH}"

# Start webserver
keepalive /opt/CodeCompass/bin/CodeCompass_webserver                           \
  --workspace "${SHARED_WORKSPACE_DIR}"                                        \
  --port "${CC_PORT}"                                                          \
  --database "${DB_CONNECTION_STRING}"
