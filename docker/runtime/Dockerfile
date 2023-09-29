ARG CC_DATABASE=sqlite
ARG INSTALL_COMMAND="apt-get install -qqy --no-install-recommends"

###############################################################################
#-----------------------------    BUILD STAGE   ------------------------------#
###############################################################################

FROM codecompass:dev as builder

ARG CC_VERSION=master
ENV CC_VERSION ${CC_VERSION}

ARG CC_DATABASE
ENV CC_DATABASE ${CC_DATABASE}

ARG CC_BUILD_TYPE=Release
ENV CC_BUILD_TYPE ${CC_BUILD_TYPE}

ARG GIT_REPO="https://github.com/Ericsson/CodeCompass.git"

RUN apt-get update -qq && \
    apt-get install --yes git

# Download CodeCompass release.
RUN git clone ${GIT_REPO} /CodeCompass
WORKDIR /CodeCompass
RUN git checkout ${CC_VERSION}

# Build CodeCompass.
RUN mkdir /CodeCompass-build && \
  cd /CodeCompass-build && \
  cmake /CodeCompass \
    -DDATABASE=$CC_DATABASE \
    -DCMAKE_INSTALL_PREFIX=/CodeCompass-install \
    -DCMAKE_BUILD_TYPE=$CC_BUILD_TYPE \
    -DWITH_AUTH="plain;ldap" && \
  make -j $(nproc) && \
  make install

###############################################################################
#--------------------------    PRODUCTION STAGE   ----------------------------#
###############################################################################

FROM ubuntu:22.04

# tzdata package is installed implicitly in the following command. This package
# sets timezone interactively during the installation process. This environment
# variable prevents this interaction.
ARG DEBIAN_FRONTEND=noninteractive
ARG INSTALL_COMMAND

ARG CC_DATABASE
ENV CC_DATABASE ${CC_DATABASE}

COPY --from=builder /packages /packages/

RUN apt-get update -qq && \
  ${INSTALL_COMMAND} /packages/odb-CodeCompass_0.1-1_amd64.deb && \
  if [ "pgsql" = "${CC_DATABASE}" ]; then \
    ${INSTALL_COMMAND} postgresql-server-dev-14; \
  else \
    ${INSTALL_COMMAND} libsqlite3-dev; \
  fi;
    
RUN set -x && apt-get update -qq \
  && ${INSTALL_COMMAND} \
    llvm-11 \
    libboost-filesystem-dev libboost-log-dev libboost-program-options-dev \
    default-jre \
    libgit2-dev \
    libssl1.1 \
    libgvc6 \
    libldap-2.4-2 \
    libmagic-dev \
    /packages/thrift-0.13-CodeCompass_0.1-1_amd64.deb \
    ctags \
    tini \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/ \
  && set +x

# Copy CodeCompass installed directory. (Change permission of the CodeCompass package.)
COPY --from=builder /CodeCompass-install /codecompass

ENV PATH="/codecompass/bin:$PATH"

ENTRYPOINT ["tini", "--"]
