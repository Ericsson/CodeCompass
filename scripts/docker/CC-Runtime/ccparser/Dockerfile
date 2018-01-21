FROM codecompass

# The following environment variables are used by the scripts that performs
# the CodeCompass related tasks. They can be overwritten by the descendant
# image(s)
# Exact documentation can be found on the beginning of the parse_project.sh.
# See the bin directory.

# Database connection arguments. They could be overwritten in the descendant
# image.
ENV PROJECT_NAME           codecompass
ENV DATABASE_HOST          localhost
ENV DATABASE_PORT          5432
ENV DATABASE_USER_NAME     postgres
ENV DATABASE_USER_PASSWORD postgres

# Directory where the particular project perfoms the build.
# PROJECT_WORKSPACE_DIR have to define in the descendant environment.

# Directory where the project specific configuration files can be stored.
ENV CONFIG_DIR             /usr/local/etc/ccparser

# Directory that must be shared between this parser and CC webserver
# For example you can use Docker volumes.
ENV CC_SHARED_WORKSPACE_DIR /data

# Where the parser scripts will be instaled.
ENV SCRIPT_DIR /usr/local/bin

# Copy all config file to the config directory.
RUN mkdir -p ${CONFIG_DIR}
COPY ./config ${CONFIG_DIR}

# TODO: Change to the nearest mirror in a better way.
#RUN \
#    cp ${CONFIG_DIR}/sources.list /etc/apt \
#    && apt-get update


RUN apt-get install --yes bear ctags cmake

# Copy all scripts to the script directory.
COPY ./bin ${SCRIPT_DIR}

# Install CC parser helper scripts
RUN chmod ugo+x \
    ${SCRIPT_DIR}/parse_project.sh \
    ${SCRIPT_DIR}/build_project.sh

WORKDIR /tmp

# Perform logging/parsing tasks. The task specialization should be placed in
# descendant image. Create ${SCRIPT_DIR}/project_specific.sh.
CMD ${SCRIPT_DIR}/parse_project.sh

