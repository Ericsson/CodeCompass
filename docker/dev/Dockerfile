FROM ubuntu:20.04

# tzdata package is installed implicitly in the following command. This package
# sets timezone interactively during the installation process. This environment
# variable prevents this interaction.
ARG DEBIAN_FRONTEND=noninteractive

# CodeCompass development dependencies.
RUN set -x && apt-get update -qq \
  && apt-get -y install --no-install-recommends \
  cmake make \
  default-jdk \
  ctags \
  gcc-9 gcc-9-plugin-dev g++-9 \
  libboost-filesystem-dev \
  libboost-log-dev \
  libboost-program-options-dev \
  libboost-regex-dev \
  libgit2-dev \
  libgraphviz-dev \
  libgtest-dev \
  libldap2-dev \
  libmagic-dev \
  libsqlite3-dev \
  libssl-dev \
  llvm-10 clang-10 llvm-10-dev libclang-10-dev \
  npm \
  thrift-compiler libthrift-dev \
  odb libodb-sqlite-dev libodb-pgsql-dev && \
  ln -s /usr/bin/gcc-9 /usr/bin/gcc && \
  ln -s /usr/bin/g++-9 /usr/bin/g++

# Build GTest.
RUN cd /usr/src/googletest && \
  mkdir build && \
  cd build && \
  cmake .. && \
  make install && \
  cd / && \
  rm -rf /usr/src/googletest/build

# Adding CodeCompass builder script.
COPY docker/dev/codecompass-build.sh /usr/local/bin

# Setting the environment.
ENV DATABASE=sqlite \
    BUILD_TYPE=Release \
    BUILD_DIR=/CodeCompass/build \
    INSTALL_DIR=/CodeCompass/install \
    SOURCE_DIR=/CodeCompass/CodeCompass \
    TEST_WORKSPACE=/CodeCompass/test_workspace \
    TEST_DB="sqlite:database=$TEST_WORKSPACE/cc_test.sqlite" \
    WITH_AUTH="plain;ldap"

ENV PATH="$INSTALL_DIR/bin:$PATH"
