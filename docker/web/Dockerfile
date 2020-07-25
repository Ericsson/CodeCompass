###############################################################################
#-----------------------------    BUILD STAGE   ------------------------------#
###############################################################################

FROM codecompass:dev as builder

ARG CC_VERSION=master
ENV CC_VERSION ${CC_VERSION}

ARG CC_DATABASE=sqlite
ENV CC_DATABASE ${CC_DATABASE}

ARG CC_BUILD_TYPE=Release
ENV CC_BUILD_TYPE ${CC_BUILD_TYPE}

RUN apt-get install -y git

# Download CodeCompass release.
RUN git clone https://github.com/Ericsson/CodeCompass.git /CodeCompass
WORKDIR /CodeCompass
RUN git checkout ${CC_VERSION}

# Build CodeCompass.
RUN mkdir /CodeCompass-build && \
  cd /CodeCompass-build && \
  cmake /CodeCompass \
    -DDATABASE=$CC_DATABASE \
    -DCMAKE_INSTALL_PREFIX=/CodeCompass-install \
    -DCMAKE_BUILD_TYPE=$CC_BUILD_TYPE && \
  make -j $(nproc) && \
  make install

###############################################################################
#--------------------------    PRODUCTION STAGE   ----------------------------#
###############################################################################

FROM ubuntu:20.04

# tzdata package is installed implicitly in the following command. This package
# sets timezone interactively during the installation process. This environment
# variable prevents this interaction.
ARG DEBIAN_FRONTEND=noninteractive

RUN set -x && apt-get update -qq \
  && apt-get install -qqy --no-install-recommends \
    llvm-7 \
    libboost-filesystem-dev libboost-log-dev libboost-program-options-dev \
    libsqlite3-dev \
    postgresql-server-dev-12 \
    default-jre \
    libgit2-dev \
    libssl1.1 \
    libgvc6 \
    libmagic-dev \
    libthrift-dev \
    libodb-sqlite-dev \
    libodb-pgsql-dev \
    ctags \
    # To switch user and exec command.
    gosu \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/ \
  && set +x

ARG CC_GID=960
ARG CC_UID=960

ENV CC_GID ${CC_GID}
ENV CC_UID ${CC_UID}

ENV TINI_VERSION v0.18.0

# Create user and group for CodeCompass.
RUN groupadd -r codecompass -g ${CC_GID} && \
    useradd -r --no-log-init -M -u ${CC_UID} -g codecompass codecompass

# Copy CodeCompass installed directory. (Change permission of the CodeCompass package.)
COPY --from=builder --chown=codecompass:codecompass /CodeCompass-install /codecompass

ENV PATH="/codecompass/bin:$PATH"

COPY --chown=codecompass:codecompass docker/web/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod a+x /usr/local/bin/entrypoint.sh

ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /tini
RUN chmod +x /tini

EXPOSE 8080

ENTRYPOINT ["/tini", "--", "/usr/local/bin/entrypoint.sh"]

CMD ["CodeCompass_webserver", "-w", "/workspace", "-p", "8080"]
