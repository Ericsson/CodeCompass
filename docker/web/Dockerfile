###############################################################################
#-----------------------------    BUILD STAGE   ------------------------------#
###############################################################################

ARG DEV_IMAGE=codecompass:dev
FROM ${DEV_IMAGE} as builder

ARG CC_VERSION=master
ENV CC_VERSION ${CC_VERSION}

ARG CC_DATABASE=sqlite
ENV CC_DATABASE ${CC_DATABASE}

ARG CC_BUILD_TYPE=Release
ENV CC_BUILD_TYPE ${CC_BUILD_TYPE}

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
FROM codecompass:dev

ARG CC_GID=960
ARG CC_UID=960

ENV CC_GID ${CC_GID}
ENV CC_UID ${CC_UID}
ENV TINI_VERSION v0.18.0

# Create user and group for CodeCompass.
RUN groupadd -r codecompass -g ${CC_GID} \
  && useradd -r --no-log-init -M -u ${CC_UID} -g codecompass codecompass

# Copy CodeCompass installed directory.
COPY --chown=codecompass:codecompass --from=builder /CodeCompass-install /codecompass 

ENV PATH="/codecompass/bin:$PATH"
ENV LD_LIBRARY_PATH="/codecompass/lib:/opt/thrift/lib:$LD_LIBRARY_PATH"

COPY --chown=codecompass:codecompass entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod a+x /usr/local/bin/entrypoint.sh


#Should be --init, make it work
ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /tini
RUN chmod +x /tini

ENTRYPOINT ["/tini", "--", "/usr/local/bin/entrypoint.sh", "CodeCompass_webserver"]


CMD ["-w", "/workspace", "-p","8080"]
