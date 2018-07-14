#!/usr/bin/env bash

set -e

# Used environment variables.
#
# ${PROJECT_NAME} - Project name
#        The "--name" option of the "CodeCompass_parser" and the "--name"
#        option of the "CodeCompass_webserver" program. The parser will create
#        a postgres database instance with this name in the postgres database.
#
# ${DATABASE_HOST} - Host [name | IP]
#        of the postgres database server.
#
# ${DATABASE_PORT} - The port
#        that the database service is listening on.
#
# ${DATABASE_USER_NAME} -  Database user name
#        that has got access to the postgres database instance.
#
# ${DATABASE_USER_PASSWORD} - Database user's password
#
# ${PROJECT_WORKSPACE_DIR} - Workspace directory
#        where the project stores its files that are necessary to build.
#
# ${CONFIG_DIR} - Configuration directory
#        where the project specific configuration files are stored
#
# ${CC_SHARED_WORKSPACE_DIR} - Workspace directory
#        where the CodeCompass parser takes its files. The CodeCompass webserver
#        will read the generated files.

# log_workspace_dir where the compilation controller json file will be stored
# by the logger phase. It must not be a subdirectory of the workspace that the
# parser uses to generate additional parsing information.
log_workspace_dir="/tmp"

# The connection string must contain the database name too that must be equal
# to the project name.
# CC issue: It must be lower case!
db_connection_string="\
pgsql:host=${DATABASE_HOST};\
port=${DATABASE_PORT};\
user=${DATABASE_USER_NAME};\
password=${DATABASE_USER_PASSWORD};\
database=${PROJECT_NAME}\
"

# Set environmet variables to run CodeCompass
export LD_LIBRARY_PATH=\
"/opt/thrift/lib:/opt/odb/lib:/opt/CodeCompass/lib:\
/opt/CodeCompass/lib/serviceplugin\
"
export PATH="/opt/CodeCompass/bin:${PATH}"
export PATH="/opt/llvm/bin:${PATH}"

# Load project specific API implementation
script_dir=$(readlink --canonicalize-existing "$(dirname "$(which "$0")")")
source "${script_dir}/project_specific.sh"

# Set up development environment of the project
# The sciptdir parameter helps to the function to find other parts of the
# project specific things.
install_project "${script_dir}" "${CONFIG_DIR}" "${PROJECT_WORKSPACE_DIR}"

compilation_database=$(get_compilation_database "${log_workspace_dir}")

if is_logging_necessary; then
    # Create compilation database for parsing. This section builds the project
    # completely.
    mkdir -p ${log_workspace_dir}
    export CC_LOGGER_GCC_LIKE="$(get_compiler_pattern)"
    CodeCompass_logger "${compilation_database}" \
        "${script_dir}/build_project.sh ${CONFIG_DIR} ${PROJECT_WORKSPACE_DIR}"
fi
filtered_compilation_database=$(filter_compilation_database "${script_dir}"    \
    "${compilation_database}")

pushd /opt/CodeCompass/bin  # Workaround for a CodeCompass bug
# --input switches pased to the parser plugins.
# ${filtered_compilation_database} used by C++ parser to index the C++ source
#   hierarchy of the program.
# ${input_dir_switches} used by file indexer plugin.
# ${CC_SHARED_WORKSPACE_DIR} will contain generated files that the CodeCompass
#   webserver will consume.
# TODO: Due to a CodeCompass bug, the directory names must not be quoted.
#   -> Now we can not use spaces in the filenames.
input_dir_switches=""
while read input_dir; do
    input_dir_switches="${input_dir_switches} --input ${input_dir}";
done < <(get_source_dir_array "${script_dir}" "${CONFIG_DIR}"                  \
  "${PROJECT_WORKSPACE_DIR}")

./keepalive ./CodeCompass_parser --workspace "${CC_SHARED_WORKSPACE_DIR}"      \
  --force --name "${PROJECT_NAME}" --database "${db_connection_string}"        \
  -j $(nproc)                                                                  \
  --input "${filtered_compilation_database}" ${input_dir_switches}             \
  --skip "dummyparser"
popd

