ARG UBUNTU_VERSION=20.04

FROM ubuntu:${UBUNTU_VERSION}

ARG UBUNTU_VERSION

# tzdata package is installed implicitly in the following command. This package
# sets timezone interactively during the installation process. This environment
# variable prevents this interaction.
ARG DEBIAN_FRONTEND=noninteractive


#Setting user and group properties
ARG CC_GID=960
ARG CC_UID=960

ENV CC_GID ${CC_GID}
ENV CC_UID ${CC_UID}

RUN groupadd --system codecompass --gid ${CC_GID} && \
    useradd --system --no-log-init --no-create-home --uid ${CC_UID} --gid codecompass codecompass


COPY docker/test/install_dependencies.sh /tmp/install_dependencies.sh
RUN chmod +x /tmp/install_dependencies.sh && \
    /tmp/install_dependencies.sh ${UBUNTU_VERSION} && \
    rm /tmp/install_dependencies.sh
    
COPY docker/dev/codecompass-build.sh /usr/local/bin

# Setting the environment.
ENV DATABASE=sqlite \
    BUILD_TYPE=Release \
    BUILD_DIR=/CodeCompass/build \
    LLVM_DIR=/usr/lib/llvm-10/cmake \
    Clang_DIR=/usr/lib/cmake/clang-10 \
    INSTALL_DIR=/CodeCompass/install \
    SOURCE_DIR=/CodeCompass/CodeCompass \
    TEST_WORKSPACE=/CodeCompass/test_workspace \
    TEST_DB="sqlite:database=$TEST_WORKSPACE/cc_test.sqlite"
    
ENV PATH=/opt/odb/bin:/opt/thrift/bin:$INSTALL_DIR/bin:$PATH

ENV GTEST_ROOT=/opt/gtest \
    CMAKE_PREFIX_PATH=/opt/thrift:/opt/odb:$CMAKE_PREFIX_PATH \
