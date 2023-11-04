FROM ubuntu:22.04

# tzdata package is installed implicitly in the following command. This package
# sets timezone interactively during the installation process. This environment
# variable prevents this interaction.
ARG DEBIAN_FRONTEND=noninteractive

# CodeCompass development dependencies.
RUN set -x && apt-get update -qq \
  && apt-get -y install --no-install-recommends \
  cmake make \
  default-jdk \
  universal-ctags \
  curl ca-certificates gnupg \
  wget \
  doxygen \
  gcc-11 gcc-11-plugin-dev g++-11 \
  libboost-filesystem-dev \
  libboost-log-dev \
  libboost-program-options-dev \
  libboost-regex-dev \
  libgit2-dev \
  libgraphviz-dev \
  libgtest-dev \
  libldap2-dev \
  libmagic-dev \
  libssl-dev \
  llvm-11 clang-11 llvm-11-dev libclang-11-dev \
  libsqlite3-dev \
  postgresql-server-dev-14 && \
  ln -s /usr/bin/gcc-11 /usr/bin/gcc && \
  ln -s /usr/bin/g++-11 /usr/bin/g++

# Copy install scripts
COPY docker/dev/install_odb.sh docker/dev/install_thrift.sh /

# Build ODB from source
RUN sh /install_odb.sh && rm /install_odb.sh

# Build Thrift 0.13 from source
RUN sh /install_thrift.sh && rm /install_thrift.sh

# Install NodeJS from NodeSource.
RUN mkdir -p /etc/apt/keyrings && \
    curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg && \
    echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_18.x nodistro main" | tee /etc/apt/sources.list.d/nodesource.list && \
    apt-get update -qq && \
    apt-get install -y nodejs && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/

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
RUN chmod +x /usr/local/bin/codecompass-build.sh

# Setting the environment.
ENV DATABASE=sqlite \
    BUILD_TYPE=Release \
    BUILD_DIR=/CodeCompass/build \
    INSTALL_DIR=/CodeCompass/install \
    SOURCE_DIR=/CodeCompass/CodeCompass \
    TEST_WORKSPACE=/CodeCompass/test_workspace \
    TEST_DB="sqlite:database=$TEST_WORKSPACE/cc_test.sqlite" \
    WITH_AUTH="plain;ldap" \
    LLVM_DIR=/usr/lib/llvm-11/cmake \
    Clang_DIR=/usr/lib/cmake/clang-11

ENV PATH="$INSTALL_DIR/bin:$PATH"
