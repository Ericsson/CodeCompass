# CodeCompass development dependecies
FROM ubuntu:bionic

RUN set -x && apt-get update -qq \
    && apt-get -y install \
    cmake make \
    gcc-7 gcc-7-plugin-dev \
    llvm-7 clang-7 llvm-7-dev libclang-7-dev \
    libsqlite3-dev \
    default-jdk \
    libssl1.0-dev \
    libgraphviz-dev \
    libmagic-dev \
    libgit2-dev \
    nodejs-dev node-gyp npm \
    ctags \
    libgtest-dev \
    # Install boost libraries.
    libboost-filesystem-dev \
    libboost-log-dev \
    libboost-program-options-dev \
    libboost-regex-dev \
    curl
# Download the Thrift source code.
ADD http://www.apache.org/dyn/mirrors/mirrors.cgi?action=download&filename=thrift/0.13.0/thrift-0.13.0.tar.gz /opt/thrift-0.13.0.tar.gz
# Build Thrift
RUN tar -xf /opt/thrift-0.13.0.tar.gz -C /opt; \
    cd /opt/thrift-0.13.0 && \
    ./configure \
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

COPY scripts/install_odb_2.5.sh /tmp/install_odb.sh

RUN chmod u+x /tmp/install_odb.sh && \
    /tmp/install_odb.sh -d /opt/odb

COPY docker/dev/codecompass-build.sh /usr/local/bin

ENV DATABASE=sqlite \
    BUILD_TYPE=Release \
    BUILD_DIR=/CodeCompass/build \
    INSTALL_DIR=/CodeCompass/install \
    SOURCE_DIR=/CodeCompass/CodeCompass \
    TEST_WORKSPACE=/CodeCompass/test_workspace \
    TEST_DB="sqlite:database=$TEST_WORKSPACE/cc_test.sqlite"

ENV PATH="$INSTALL_DIR/bin:/opt/odb/bin:$PATH"
ENV CMAKE_PREFIX_PATH="/opt/odb:$PATH"
ENV LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH"
