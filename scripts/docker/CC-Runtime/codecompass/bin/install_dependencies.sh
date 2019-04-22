#!/usr/bin/env bash

packages_to_install=(                                                          \
    openjdk-8-jdk                                                              \
    libgvc6                                                                    \
    libpq5                                                                     \
)

if [[ "${1}" == "16.04" ]]; then
    packages_to_install+=(                                                     \
        libboost-filesystem1.58.0                                              \
        libboost-log1.58.0                                                     \
        libboost-program-options1.58.0                                         \
        libboost-thread1.58.0                                                  \
        libgit2-24                                                             \
    )
elif [[ "${1}" == "18.04" ]]; then
    packages_to_install+=(                                                     \
        libboost-filesystem1.65.1                                              \
        libboost-log1.65.1                                                     \
        libboost-program-options1.65.1                                         \
        libboost-thread1.65.1                                                  \
        libgit2-26                                                             \
        libodb-sqlite-2.4                                                      \
        libodb-pgsql-2.4                                                       \
    )
fi

apt-get install --yes ${packages_to_install[@]}
