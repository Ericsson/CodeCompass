#!/usr/bin/env bash

packages_to_install=(                                                          \
    autoconf                                                                   \
    cmake                                                                      \
    g++                                                                        \
    make                                                                       \
)

if [[ "${1}" == "16.04" ]]; then
    packages_to_install+=(libcurl3)
fi
if [[ "${1}" == "18.04" ]]; then
    packages_to_install+=(libcurl4)
fi

apt-get install --yes ${packages_to_install[@]}
