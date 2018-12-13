FROM ccparser

ARG project_name
ARG database_host
ARG database_port
ARG database_user_name
ARG database_users_password
ARG project_workspace_dir
ARG shared_workspace_dir

# Database connection arguments
ENV PROJECT_NAME            ${project_name:-xerces}
ENV DATABASE_HOST           ${database_host:-localhost}
ENV DATABASE_PORT           ${database_port:-5432}
ENV DATABASE_USER_NAME      ${database_user_name:-postgres}
ENV DATABASE_USER_PASSWORD  ${database_users_password:-postgres}

# Directory where this project perfoms the build
# It can be overwritten in build time with build-arg or run-time by setting
# environment variable in the command line.
ENV PROJECT_WORKSPACE_DIR   ${project_workspace_dir:-${PROJECT_WORKSPACE_DIR}}

# Directory that must be shared between this parser and CC webserver
ENV CC_SHARED_WORKSPACE_DIR ${shared_workspace_dir:-${CC_SHARED_WORKSPACE_DIR}}

# Install scripts to the script directory, including project specific things
COPY "./bin/filterbuildcmds.py" "${SCRIPT_DIR}"
COPY "./bin/project_specific.sh" "${SCRIPT_DIR}"

WORKDIR "/root"
COPY "./bin/install_dependencies.sh" "./bin/"
RUN chmod ugo+x "${SCRIPT_DIR}/filterbuildcmds.py"                             \
    && chmod u+x "./bin/install_dependencies.sh"

# Install developer tools for xerces
RUN apt-get update \
    && "./bin/install_dependencies.sh"

# Prepare directory for indexer plugin
RUN mkdir --parent --mode=a+rwx "${CC_SHARED_WORKSPACE_DIR}"

WORKDIR /tmp
