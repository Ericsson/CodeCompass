#!/usr/bin/env bash

set -e

function cleanup() {
    echo "Cleaning up Thrift temporaries."
    if  [[ -n "${THRIFT_BUILD_DIR}" ]]; then
        rm --recursive --force "${THRIFT_BUILD_DIR}"
    fi
}

trap cleanup EXIT

function usage() {
    echo "${0} [-h] -t <thrift version> [-p]"
    echo "  -h  Print this usage information. Optional."
    echo "  -t  Thrift version. Mandatory. For example '0.11.0'."
    echo "  -p  Additional PATH components."
}

while getopts "ht:p:" OPTION; do
    case ${OPTION} in
        h)
            usage
            exit 0
            ;;
        t)
            THRIFT_VERSION="${OPTARG}"
            ;;
        p)
            ADDITIONAL_PATH="${OPTARG}"
            ;;
        *)
            usage >&2
            exit 1
            ;;
    esac
done

if [[ -z "${THRIFT_VERSION}" ]]; then
    echo "Thrift version should be defined." >&2
    usage
    exit 1
fi

if [[ -n "${ADDITIONAL_PATH}" ]]; then
    export PATH="${ADDITIONAL_PATH}":"${PATH}"
fi

THRIFT_ARCHIVE_NAME="thrift-${THRIFT_VERSION}.tar.gz"
THRIFT_BUILD_DIR="/tmp/thrift"
THRIFT_SRC_DIR="${THRIFT_BUILD_DIR}/thrift"
THRIFT_INSTALL_DIR="/opt/thrift"
FAKE_JAVA_INSTALL_DIR="/usr/local/lib"
JAVA_LIB_INSTALL_DIR="${THRIFT_INSTALL_DIR}/lib/java"

mkdir --parents "${THRIFT_SRC_DIR}"
wget --no-verbose \
  "http://xenia.sote.hu/ftp/mirrors/www.apache.org/thrift/${THRIFT_VERSION}/${THRIFT_ARCHIVE_NAME}" \
  --output-document="${THRIFT_BUILD_DIR}/${THRIFT_ARCHIVE_NAME}"
tar --extract --gunzip --file="${THRIFT_BUILD_DIR}/${THRIFT_ARCHIVE_NAME}"     \
    --directory="${THRIFT_SRC_DIR}" --strip-components=1
rm "${THRIFT_BUILD_DIR}/${THRIFT_ARCHIVE_NAME}"

configure_cmd=("./configure" "--prefix=${THRIFT_INSTALL_DIR}"                  \
  "JAVA_PREFIX=${THRIFT_INSTALL_DIR}"                                          \
  "--enable-libtool-lock" "--enable-tutorial=no" "--enable-tests=no"           \
  "--with-libevent" "--with-zlib" "--without-nodejs" "--without-lua"           \
  "--without-ruby" "--without-csharp" "--without-erlang" "--without-perl"      \
  "--without-php" "--without-php_extension" "--without-dart"                   \
  "--without-haskell" "--without-go" "--without-rs" "--without-haxe"           \
  "--without-dotnetcore" "--without-d" "--without-qt4" "--without-qt5"         \
  "--with-java")

# Workaround. Downloading java components during java build needs it.
if [[ ! -z ${http_proxy} ]]; then
    proxy_elements=(${http_proxy//:/ })
    proxy_protocol="${proxy_elements[0]}"
    proxy_host="${proxy_elements[1]:2}"
    proxy_port="${proxy_elements[2]}"

    maven_config_dir="${HOME}/.m2"
    mkdir --parents "${maven_config_dir}"
    cat << EOF > "${maven_config_dir}/settings.xml"
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0"
      xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
      xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0
                          https://maven.apache.org/xsd/settings-1.0.0.xsd">
    <proxies>
        <proxy>
            <active>true</active>
            <protocol>${proxy_protocol}</protocol>
            <host>${proxy_host}</host>
            <port>${proxy_port}</port>
            <username></username>
            <password></password>
            <nonProxyHosts></nonProxyHosts>
        </proxy>
    </proxies>
</settings>
EOF
    ANT_FLAGS="-Dproxy.enabled=1 -Dproxy.host=${proxy_host} -Dproxy.port=${proxy_port}"
    configure_cmd+=("ANT_FLAGS=${ANT_FLAGS}")
fi

pushd "${THRIFT_SRC_DIR}"

"${configure_cmd[@]}"

# Workaround if we are behind a proxy.
# Configuring with ANT_FLAGS described on
# https://thrift.apache.org/lib/java does not work.
if [[ ! -z ${http_proxy} ]]; then
    pushd "lib/java"
    ant "${ANT_FLAGS}"
    popd
fi

make --jobs="$(nproc)" install

popd

# Workaround.
# JAVA_PREFIX causes that the java libraries will be installed in
# $JAVA_PREFIX/usr/local/lib
# CodeCompass needs java libs together with c++ libs. We follow the debian
# library layout, so finally java libs should move to $PREFIX/lib/java.
mv "${THRIFT_INSTALL_DIR}${FAKE_JAVA_INSTALL_DIR}" "${JAVA_LIB_INSTALL_DIR}"
rm --recursive --force "${THRIFT_INSTALL_DIR}/usr"
