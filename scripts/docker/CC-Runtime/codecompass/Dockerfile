FROM compass-devel AS code-compass-installer

ARG compass_source_url

WORKDIR /root
COPY ./bin ./bin
RUN chmod u+x ./bin/make_compass.sh
RUN ./bin/make_compass.sh ${compass_source_url}


FROM ubuntu:16.04 AS code-compass

RUN \
    apt-get update \
    && apt-get install --yes \
    openjdk-8-jdk \
    libboost-filesystem1.58.0 \
    libboost-log1.58.0 \
    libboost-program-options1.58.0 \
    libboost-thread1.58.0 \
    libgit2-24 \
    libgvc6 \
    libpq5

COPY --from=code-compass-installer /opt /opt

CMD ["/bin/bash"]
