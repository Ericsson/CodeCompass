#!/usr/bin/env bash

declare -a packages_to_install

packages_to_install=(                                                          \
    "ant"                                                                      \
    "autoconf"                                                                 \
    "automake"                                                                 \
    "bison"                                                                    \
    "cmake"                                                                    \
    "flex"                                                                     \
    "g++-5"                                                                    \
    "gcc-5-plugin-dev"                                                         \
    "git"                                                                      \
    "libboost-all-dev"                                                         \
    "libcutl-dev"                                                              \
    "libevent-dev"                                                             \
    "libexpat1-dev"                                                            \
    "libgit2-dev"                                                              \
    "libgraphviz-dev"                                                          \
    "libgtest-dev"                                                             \
    "libmagic-dev"                                                             \
    "libpq-dev"                                                                \
    "libsqlite3-dev"                                                           \
    "libssl-dev"                                                               \
    "libtool"                                                                  \
    "lsb-release"                                                              \
    "make"                                                                     \
    "pkg-config"                                                               \
    "wget"                                                                     \
    "xz-utils"                                                                 \
    "zlib1g-dev"                                                               \
)

if [[ "${1}" == "16.04" ]]; then
    packages_to_install+=("nodejs-legacy")
elif [[ "${1}" == "18.04" ]]; then
    packages_to_install+=("odb" "libodb-dev" "libodb-sqlite-dev"               \
                          "libodb-pgsql-dev" "nodejs")
fi

apt-get install --yes "apt-utils"

# Workaround. This single step provides that the JDK 8 will be installed only.
apt-get install --yes "openjdk-8-jdk-headless"

# Install packages that necessary for build CodeCompass.
apt-get install --yes ${packages_to_install[@]}

# Workaround. This single step prevent unwanted remove of npm.
apt-get install --yes "npm"
