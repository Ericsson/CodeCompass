ARG UBUNTU_VERSION=16.04

FROM compass-devel AS code-compass-installer

ARG compass_source_url
ARG compass_branch

WORKDIR /root
COPY ./bin/make_compass.sh ./bin/
RUN chmod u+x ./bin/make_compass.sh
RUN ./bin/make_compass.sh ${compass_source_url} ${compass_branch}


FROM ubuntu:${UBUNTU_VERSION} AS code-compass

ARG UBUNTU_VERSION

WORKDIR /root
COPY ./bin/install_dependencies.sh ./bin/
RUN chmod u+x ./bin/install_dependencies.sh

RUN \
    apt-get update \
    && ./bin/install_dependencies.sh ${UBUNTU_VERSION}

COPY --from=code-compass-installer /opt /opt

CMD ["/bin/bash"]
