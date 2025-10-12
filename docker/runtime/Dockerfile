ARG CC_DATABASE=sqlite

###############################################################################
#-----------------------------    BUILD STAGE   ------------------------------#
###############################################################################

FROM codecompass:dev as builder

ARG CC_REPO_URL="https://github.com/Ericsson/CodeCompass.git"
ENV CC_REPO_URL ${CC_REPO_URL}

ARG CC_VERSION=master
ENV CC_VERSION ${CC_VERSION}

ARG CC_DATABASE
ENV CC_DATABASE ${CC_DATABASE}

ARG CC_BUILD_TYPE=Release
ENV CC_BUILD_TYPE ${CC_BUILD_TYPE}

RUN apt-get update -qq && \
    apt-get install --yes git

# Download CodeCompass release.
RUN git clone ${CC_REPO_URL} /CodeCompass
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

ARG CC_DATABASE
ENV CC_DATABASE ${CC_DATABASE}

RUN set -x && apt-get update -qq && \
    apt-get install -qq --yes --no-install-recommends \
    curl ca-certificates gnupg \
    wget \
    llvm-15 \
    dotnet-sdk-8.0 \
    libboost-filesystem-dev libboost-log-dev libboost-program-options-dev \
    default-jre \
    libgit2-dev \
    libssl3 \
    libgvc6 \
    libldap-2.5-0 \
    libmagic-dev \
    libthrift-dev \
    universal-ctags \
    gcc-11 g++-11 \
    tini && \
    ln -s /usr/bin/gcc-11 /usr/bin/gcc && \
    ln -s /usr/bin/g++-11 /usr/bin/g++ && \
    if [ "pgsql" = "${CC_DATABASE}" ]; then \
      apt-get install -qq --yes --no-install-recommends \
      libpq5 \
      postgresql-server-dev-14; \
    else \
      apt-get install -qq --yes --no-install-recommends \
      libsqlite3-dev; \
    fi && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/ && \
    set +x

# Copy ODB install directory.
COPY --from=builder /opt/odb /opt/odb

# Copy CodeCompass install directory.
COPY --from=builder /CodeCompass-install /codecompass

ENV PATH="/codecompass/bin:/opt/odb/bin:$PATH"

ENTRYPOINT ["tini", "--"]
