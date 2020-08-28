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
    llvm-10 \
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
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/ \
  && set +x

ENV TINI_VERSION v0.18.0

# Copy CodeCompass installed directory. (Change permission of the CodeCompass package.)
COPY --from=builder /CodeCompass-install /codecompass

ENV PATH="/codecompass/bin:$PATH"

ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /tini
RUN chmod +x /tini

ENTRYPOINT ["/tini", "--"]

