###############################################################################
#-----------------------------    BUILD STAGE   ------------------------------#
###############################################################################

FROM ubuntu:bionic as builder

ARG CC_VERSION=master
ENV CC_VERSION ${CC_VERSION}

ARG CC_DATABASE=sqlite
ENV CC_DATABASE ${CC_DATABASE}

ARG CC_BUILD_TYPE=Release
ENV CC_BUILD_TYPE ${CC_BUILD_TYPE}

RUN set -x && apt-get update -qq \
  && apt-get install -qqy \
    cmake make \
    llvm-7 clang-7 llvm-7-dev libclang-7-dev \
    libboost-all-dev \
    odb libodb-sqlite-dev libodb-pgsql-dev \
    libsqlite3-dev \
    default-jdk ant\
    libssl1.0-dev \
    libgraphviz-dev \
    libmagic-dev \
    libgit2-dev \
    nodejs-dev node-gyp npm \
    ctags \
    wget \
    git \
    libgtest-dev \
    && \
    # Build thrift.
    wget "http://www.apache.org/dyn/mirrors/mirrors.cgi?action=download&filename=thrift/0.12.0/thrift-0.12.0.tar.gz" -O /opt/thrift-0.12.0.tar.gz; \
    tar -xf /opt/thrift-0.12.0.tar.gz -C /opt; \
    cd /opt/thrift-0.12.0; \
    ./configure --prefix=/opt/thrift --silent; \
    cd /; \
    make -C /opt/thrift-0.12.0 -j4 --silent install; \
    rm -rf /opt/thrift-0.12.0 /opt/thrift-0.12.0.tar.gz; \
    # Build GTest.
    cd /usr/src/googletest/ && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make install && \
    cd / && \
    rm -rf /usr/src/googletest/build

ENV PATH="/opt/thrift/bin:$PATH"
ENV CMAKE_PREFIX_PATH="/opt/thrift:$CMAKE_PREFIX_PATH"

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
  make && \
  make install

###############################################################################
#--------------------------    PRODUCTION STAGE   ----------------------------#
###############################################################################
FROM ubuntu:bionic

RUN set -x && apt-get update -qq \
  && apt-get install -qqy --no-install-recommends \
    llvm-7 \
    libboost-filesystem-dev libboost-log-dev libboost-program-options-dev \
    odb libodb-sqlite-dev libodb-pgsql-dev \
    libsqlite3-dev \
    default-jre \
    libgit2-dev \
    libssl1.0.0 \
    libgvc6 \
    # To switch user and exec command.
    gosu \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/ \
  && set +x

# Copy CodeCompass installed directory.
COPY --from=builder /CodeCompass-install /codecompass

# Copy thrift installed directory.
COPY --from=builder /opt/thrift /opt/thrift

ARG CC_GID=960
ARG CC_UID=960

ENV CC_GID ${CC_GID}
ENV CC_UID ${CC_UID}

ENV TINI_VERSION v0.18.0

# Create user and group for CodeCompass.
RUN groupadd -r codecompass -g ${CC_GID} \
  && useradd -r --no-log-init -M -u ${CC_UID} -g codecompass codecompass

# Change permission of the CodeCompass package.
RUN chown codecompass:codecompass /codecompass

ENV PATH="/codecompass/bin:$PATH"
ENV LD_LIBRARY_PATH="/codecompass/lib:/opt/thrift/lib:$LD_LIBRARY_PATH"

COPY ./entrypoint.sh /usr/local/bin/
RUN chmod a+x /usr/local/bin/entrypoint.sh \
  && chown codecompass:codecompass /usr/local/bin/entrypoint.sh

ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /tini
RUN chmod +x /tini

EXPOSE 8080

ENTRYPOINT ["/tini", "--", "/usr/local/bin/entrypoint.sh"]

CMD ["CodeCompass_webserver", "-w", "/workspace", "-p", "8080"]
