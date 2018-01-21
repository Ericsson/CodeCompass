FROM ubuntu:16.04

RUN \
    apt-get update \
    && apt-get install --yes \
    openjdk-8-jdk-headless \
    libboost-filesystem1.58.0 \
    libboost-log1.58.0 \
    libboost-program-options1.58.0 \
    libboost-thread1.58.0 \
    pkg-config \
    ant \
    make \
    autoconf \
    automake \
    libtool \
    flex \
    bison \
    libssl-dev \
    git \
    libgit2-dev \
    libboost-all-dev \
    libevent-dev \
    wget \
    zlib1g-dev \
    libpq-dev \
    gcc-5-plugin-dev \
    libcutl-dev \
    cmake \
    libgraphviz-dev \
    libgtest-dev \
    libmagic-dev \
    libpq-dev \
    libsqlite3-dev \
    xz-utils \
    lsb-release

WORKDIR /root
COPY ./bin ./bin
RUN \
    chmod u+x \
    ./bin/install_gtest.sh \
    ./bin/install_thrift.sh \
    ./bin/install_dbaccess.sh \
    ./bin/install_llvm.sh \
    ./bin/install_npm.sh

RUN \
    ./bin/install_gtest.sh \
    && ./bin/install_thrift.sh -t "0.10.0" \
    && ./bin/install_dbaccess.sh \
    && ./bin/install_llvm.sh -l "3.8.1" \
    && wget -qO- \
    https://raw.githubusercontent.com/creationix/nvm/v0.33.8/install.sh \
    | NVM_DIR=/opt/nvm /bin/bash \
    && ./bin/install_npm.sh

RUN \
    mv /root/bin/fetchcc.sh \
       /root/bin/configurecc.sh \
       /root/bin/buildcc.sh \
       /root/bin/builder_config.sh \
       /usr/local/bin \
    && chmod ugo+x \
    /usr/local/bin/fetchcc.sh \
    /usr/local/bin/configurecc.sh \
    /usr/local/bin/buildcc.sh

WORKDIR /tmp

