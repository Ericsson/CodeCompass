ARG UBUNTU_VERSION=16.04

FROM ubuntu:${UBUNTU_VERSION}

ARG UBUNTU_VERSION

LABEL com.ericsson.codecompass.ubuntu.version="${UBUNTU_VERSION}"

RUN apt-get update                                                             \
  && DEBIAN_FRONTEND=noninteractive apt-get install --yes locales

RUN sed --in-place --expression='s/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/'     \
    /etc/locale.gen                                                            \
  && dpkg-reconfigure --frontend=noninteractive locales                        \
  && update-locale LANG=en_US.UTF-8

ENV LANG en_US.UTF-8

WORKDIR /root
COPY ./bin ./bin
RUN chmod u+x \
    ./bin/install_gtest.sh                                                     \
    ./bin/install_thrift.sh                                                    \
    ./bin/install_dbaccess.sh                                                  \
    ./bin/install_llvm.sh                                                      \
    ./bin/install_dependencies.sh

RUN ./bin/install_dependencies.sh ${UBUNTU_VERSION}

RUN ./bin/install_gtest.sh                                                     \
  && ./bin/install_thrift.sh -t "0.11.0"                                       \
  && ./bin/install_dbaccess.sh ${UBUNTU_VERSION}                               \
  && ./bin/install_llvm.sh -l "7.0.0"

RUN mv \
    /root/bin/fetchcompass.sh                                                  \
    /root/bin/configurecompass.sh                                              \
    /root/bin/buildcompass.sh                                                  \
    /root/bin/builder_config.sh                                                \
    /usr/local/bin                                                             \
  && chmod ugo+x                                                               \
    /usr/local/bin/fetchcompass.sh                                             \
    /usr/local/bin/configurecompass.sh                                         \
    /usr/local/bin/buildcompass.sh

WORKDIR /tmp
