###############################################################################
#----------------------------   IMPORT RUNTIME   -----------------------------#
###############################################################################

FROM codecompass:runtime as runtime

###############################################################################
#------------------------    EXECUTABLE CONTAINER   --------------------------#
###############################################################################

FROM ubuntu:22.04

# tzdata package is installed implicitly in the following command. This package
# sets timezone interactively during the installation process. This environment
# variable prevents this interaction.
ARG DEBIAN_FRONTEND=noninteractive

RUN set -x && apt-get update -qq \
  && apt-get install -qqy --no-install-recommends \
    llvm-15 \
    libboost-filesystem-dev libboost-log-dev libboost-program-options-dev \
    libsqlite3-dev \
    postgresql-server-dev-14 \
    default-jre \
    libgit2-dev \
    libldap-2.5-0 \
    libssl3 \
    libgvc6 \
    libthrift-dev \
    libpq5 \
    # To switch user and exec command.
    gosu \
    tini \
    curl ca-certificates gnupg \
    wget \
    gcc-11 g++-11 \
  && ln -s /usr/bin/gcc-11 /usr/bin/gcc \
  && ln -s /usr/bin/g++-11 /usr/bin/g++ \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/ \
  && set +x

# Install NodeJS from NodeSource.
RUN mkdir -p /etc/apt/keyrings && \
    curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg && \
    echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_18.x nodistro main" | tee /etc/apt/sources.list.d/nodesource.list && \
    apt-get update -qq && \
    apt-get install -y nodejs && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/

# Copy ODB install directory.
COPY --from=runtime /opt/odb /opt/odb

ARG CC_GID=960
ARG CC_UID=960

ENV CC_GID ${CC_GID}
ENV CC_UID ${CC_UID}

# Create user and group for CodeCompass.
RUN groupadd --system codecompass --gid ${CC_GID} && \
    useradd --system --no-log-init --no-create-home --uid ${CC_UID} --gid codecompass codecompass

# Copy CodeCompass install directory. (Change permission of the CodeCompass package.)
# TODO: only the webserver's binaries should be included in this image.
COPY --from=runtime --chown=codecompass:codecompass /codecompass /codecompass

ENV PATH="/codecompass/bin:/opt/odb/bin:$PATH"

COPY --chown=codecompass:codecompass docker/web/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod a+x /usr/local/bin/entrypoint.sh


EXPOSE 8080

ENTRYPOINT ["tini", "--", "/usr/local/bin/entrypoint.sh"]

CMD ["CodeCompass_webserver", "-w", "/workspace", "-p", "8080"]
