#!/usr/bin/env bash

declare -a packages_to_install

packages_to_install=(                                                          \
    "ant"                                                                      \
    "autoconf"                                                                 \
    "automake"                                                                 \
    "bison"                                                                    \
    "clang-7"                                                                  \
    "flex"                                                                     \
    "git"                                                                      \
    "libboost-all-dev"                                                         \
    "libclang-7-dev"                                                           \
    "libcutl-dev"                                                              \
    "libevent-dev"                                                             \
    "libexpat1-dev"                                                            \
    "libgit2-dev"                                                              \
    "libgraphviz-dev"                                                          \
    "llvm-7-dev"                                                               \
    "libmagic-dev"                                                             \
    "libodb-dev"                                                               \
    "libodb-pgsql-dev"                                                         \
    "libodb-sqlite-dev"                                                        \
    "libpq-dev"                                                                \
    "libsqlite3-dev"                                                           \
    "libssl-dev"                                                               \
    "libtool"                                                                  \
    "make"                                                                     \
    "odb"                                                                      \
    "pkg-config"                                                               \
    "wget"                                                                     \
    "xz-utils"                                                                 \
    "zlib1g-dev"                                                               \
)

declare running_ubuntu_codename="$(lsb_release --codename --short)"
if [[ "${running_ubuntu_codename}" == "xenial" ]]; then
    packages_to_install+=("nodejs-legacy")
elif [[ "${running_ubuntu_codename}" == "bionic" ]]; then
    packages_to_install+=( "nodejs")
else
    echo "Unsupported ubuntu release" 2>&1
    exit 1
fi

# Workaround. This single step provides that gtest installed properly on
# xenial.
apt-get install --yes  "libgtest-dev" "cmake"

# Workaround. This single step provides that the JDK 8 will be installed only.
apt-get install --yes "openjdk-8-jdk-headless"

# Install packages that necessary for build CodeCompass.
apt-get install --yes "${packages_to_install[@]}"

# Workaround. This single step prevent unwanted remove of npm.
apt-get install --yes "npm"
