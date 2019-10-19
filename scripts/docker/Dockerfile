FROM ubuntu:bionic

RUN apt-get update && apt-get -y install \
  cmake make \
  llvm-7 clang-7 llvm-7-dev libclang-7-dev \
  libboost-all-dev \
  odb libodb-sqlite-dev libodb-pgsql-dev \
  libsqlite3-dev \
  default-jdk ant\
  libssl-dev \
  libgraphviz-dev \
  libmagic-dev \
  libgit2-dev \
  nodejs \
  ctags \
  wget \
  git \
  && \
  wget "http://www.apache.org/dyn/mirrors/mirrors.cgi?action=download&filename=thrift/0.12.0/thrift-0.12.0.tar.gz" -O /opt/thrift-0.12.0.tar.gz; \
  tar -xf /opt/thrift-0.12.0.tar.gz -C /opt; \
  cd /opt/thrift-0.12.0; \
  ./configure --prefix=/opt/thrift --silent; \
  cd /; \
  make -C /opt/thrift-0.12.0 -j4 --silent install; \
  rm -rf /opt/thrift-0.12.0 /opt/thrift-0.12.0.tar.gz

COPY codecompass-build.sh /usr/local/bin

ENV DATABASE=pgsql BUILD_TYPE=Release

ENTRYPOINT ["codecompass-build.sh"]
