FROM ubuntu:16.04

RUN apt-get update \
  && apt-get install --yes \
    ant \
    autoconf \
    automake \
    bison \
    cmake \
    flex \
    gcc-5-plugin-dev \
    git \
    libboost-all-dev \
    libboost-filesystem1.58.0 \
    libboost-log1.58.0 \
    libboost-program-options1.58.0 \
    libboost-thread1.58.0 \
    libcutl-dev \
    libevent-dev \
    libgit2-dev \
    libgraphviz-dev \
    libgtest-dev \
    libmagic-dev \
    libpq-dev \
    libsqlite3-dev \
    libssl-dev \
    libtool \
    lsb-release \
    make \
    openjdk-8-jdk-headless \
    pkg-config \
    wget \
    xz-utils \
    zlib1g-dev

WORKDIR /root
COPY ./bin ./bin
RUN chmod u+x \
    ./bin/install_gtest.sh \
    ./bin/install_thrift.sh \
    ./bin/install_dbaccess.sh \
    ./bin/install_llvm.sh \
    ./bin/install_nvm.sh

RUN ./bin/install_gtest.sh \
  && ./bin/install_thrift.sh -t "0.10.0" \
  && ./bin/install_dbaccess.sh \
  && ./bin/install_llvm.sh -l "6.0.0" \
  && wget -qO- \
    https://raw.githubusercontent.com/creationix/nvm/v0.33.8/install.sh \
    | NVM_DIR=/opt/nvm /bin/bash \
  && ./bin/install_nvm.sh

RUN mv \
    /root/bin/fetchcompass.sh \
    /root/bin/configurecompass.sh \
    /root/bin/buildcompass.sh \
    /root/bin/builder_config.sh \
    /usr/local/bin \
  && chmod ugo+x \
    /usr/local/bin/fetchcompass.sh \
    /usr/local/bin/configurecompass.sh \
    /usr/local/bin/buildcompass.sh

WORKDIR /tmp

