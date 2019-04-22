#!/usr/bin/env bash

packages_to_install=(                                                          \
    ant                                                                        \
    autoconf                                                                   \
    automake                                                                   \
    bison                                                                      \
    cmake                                                                      \
    flex                                                                       \
    g++-5                                                                      \
    gcc-5-plugin-dev                                                           \
    git                                                                        \
    libboost-all-dev                                                           \
    libcutl-dev                                                                \
    libevent-dev                                                               \
    libexpat1-dev                                                              \
    libgit2-dev                                                                \
    libgraphviz-dev                                                            \
    libgtest-dev                                                               \
    libmagic-dev                                                               \
    libpq-dev                                                                  \
    libsqlite3-dev                                                             \
    libssl-dev                                                                 \
    libtool                                                                    \
    lsb-release                                                                \
    make                                                                       \
    pkg-config                                                                 \
    wget                                                                       \
    xz-utils                                                                   \
    zlib1g-dev                                                                 \
)

if [[ "${1}" == "16.04" ]]; then
    packages_to_install+=("npm" "nodejs-legacy")
fi
if [[ "${1}" == "18.04" ]]; then
    packages_to_install+=("nodejs" "odb" "libodb-dev" "libodb-sqlite-dev"      \
                          "libodb-pgsql-dev")
fi

apt-get install --yes apt-utils

# Workaround. This single step provides that only the JDK 8 will be installed.
apt-get install --yes openjdk-8-jdk-headless

apt-get install --yes ${packages_to_install[@]}
