# CodeCompass development dependencies
FROM ubuntu:bionic

RUN set -x && apt-get update -qq \
    && apt-get -y install \
    cmake make \
    curl ctags \
    default-jdk \
    gcc-7 gcc-7-plugin-dev \
    libboost-filesystem-dev \
    libboost-log-dev \
    libboost-program-options-dev \
    libboost-regex-dev \
    libgit2-dev \
    libgraphviz-dev \
    libgtest-dev \
    libmagic-dev \
    libsqlite3-dev \
    libssl1.0-dev \
    llvm-7 clang-7 llvm-7-dev libclang-7-dev \
    nodejs-dev node-gyp npm

# Download the Thrift source code.
ADD http://www.apache.org/dyn/mirrors/mirrors.cgi?action=download&filename=thrift/0.13.0/thrift-0.13.0.tar.gz /opt/thrift-0.13.0.tar.gz
# Build Thrift
RUN tar -xf /opt/thrift-0.13.0.tar.gz -C /opt; \
    cd /opt/thrift-0.13.0 && \
    ./configure --prefix=/opt/thrift_install \
    --silent --without-python                                        \
    --enable-libtool-lock --enable-tutorial=no --enable-tests=no     \
    --with-libevent --with-zlib --without-nodejs --without-lua       \
    --without-ruby --without-csharp --without-erlang --without-perl  \
    --without-php --without-php_extension --without-dart             \
    --without-haskell --without-go --without-rs --without-haxe       \
    --without-dotnetcore --without-d --without-qt4 --without-qt5;    \
    cd /; \
    make -C /opt/thrift-0.13.0 -j $(nproc) --silent install; \
    rm -rf /opt/thrift-0.13.0
# Build GTest.
RUN cd /usr/src/googletest/ && \
     mkdir build && \
     cd build && \
     cmake .. && \
     make install && \
     cd / && \
     rm -rf /usr/src/googletest/build

# Download build2 toolchain install script.
ADD https://download.build2.org/0.12.0/build2-install-0.12.0.sh /opt/build2-install-0.12.0.sh
# Build build2, then ODB.
RUN cd /opt/ && \
    sh build2-install-0.12.0.sh --yes --trust yes; \
    mkdir /opt/odb_build && \
    cd /opt/odb_build && \
    bpkg create --quiet --jobs $(nproc) cc \
    config.cxx=g++ \
    config.cc.coptions=-O3 \
    config.bin.rpath=/opt/odb_install/lib \
    config.install.root=/opt/odb_install && \
    bpkg add https://pkg.cppget.org/1/beta --trust-yes && \
    bpkg fetch --trust-yes && \
    bpkg build odb --yes && \
    bpkg build libodb --yes && \
    bpkg build libodb-sqlite --yes && \
    bpkg build libodb-pgsql --yes && \
    bpkg install --all --recursive; \
    rm -rf /opt/build2* && \
    rm -rf /opt/odb_build

COPY docker/dev/codecompass-build.sh /usr/local/bin

ENV DATABASE=sqlite \
    BUILD_TYPE=Release \
    BUILD_DIR=/CodeCompass/build \
    INSTALL_DIR=/CodeCompass/install \
    SOURCE_DIR=/CodeCompass/CodeCompass \
    TEST_WORKSPACE=/CodeCompass/test_workspace \
    TEST_DB="sqlite:database=$TEST_WORKSPACE/cc_test.sqlite"

ENV PATH="$INSTALL_DIR/bin:/opt/thrift_install/bin:/opt/odb_install/bin:$PATH"
ENV CMAKE_PREFIX_PATH="/opt/thrift_install:/opt/odb_install:$CMAKE_PREFIX_PATH"
ENV LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH"
