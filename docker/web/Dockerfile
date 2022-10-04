###############################################################################
#----------------------------   IMPORT RUNTIME   -----------------------------#
###############################################################################

FROM codecompass:runtime as runtime

###############################################################################
#------------------------    EXECUTABLE CONTAINER   --------------------------#
###############################################################################

FROM ubuntu:20.04

# tzdata package is installed implicitly in the following command. This package
# sets timezone interactively during the installation process. This environment
# variable prevents this interaction.
ARG DEBIAN_FRONTEND=noninteractive

RUN set -x && apt-get update -qq \
  && apt-get install -qqy --no-install-recommends \
    wget \
    llvm-10 \
    libboost-filesystem-dev libboost-log-dev libboost-program-options-dev \
    libsqlite3-dev \
    postgresql-server-dev-12 \
    default-jre \
    libgit2-dev \
    libldap-2.4-2 \
    libssl1.1 \
    libgvc6 \
    libthrift-dev \
    libodb-sqlite-dev \
    libodb-pgsql-dev \
    # To switch user and exec command.
    gosu \
    tini \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/ \
  && set +x

# Install .NET 6.
RUN wget https://packages.microsoft.com/config/ubuntu/20.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb && \
  dpkg -i packages-microsoft-prod.deb && \
  apt-get update; \
  apt-get install -y apt-transport-https && \
  apt-get update && \
  apt-get install -y dotnet-sdk-6.0

ARG CC_GID=960
ARG CC_UID=960

ENV CC_GID ${CC_GID}
ENV CC_UID ${CC_UID}


# Create user and group for CodeCompass.
RUN groupadd --system codecompass --gid ${CC_GID} && \
    useradd --system --no-log-init --no-create-home --uid ${CC_UID} --gid codecompass codecompass

# Copy CodeCompass installed directory. (Change permission of the CodeCompass package.)
# TODO: only the webserver's binaries should be included in this image.
COPY --from=runtime --chown=codecompass:codecompass /codecompass /codecompass

ENV PATH="/codecompass/bin:$PATH"

COPY --chown=codecompass:codecompass docker/web/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod a+x /usr/local/bin/entrypoint.sh


EXPOSE 8080

ENTRYPOINT ["tini", "--", "/usr/local/bin/entrypoint.sh"]

CMD ["CodeCompass_webserver", "-w", "/workspace", "-p", "8080"]
