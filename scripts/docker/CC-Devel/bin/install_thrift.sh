#!/usr/bin/env bash

set -e

function cleanup() {
    echo "Cleaning up Thrift temporaries."
    if  [[ -n "${thrift_build_dir}" ]]; then
        rm --recursive --force "${thrift_build_dir}"
    fi
}

trap cleanup EXIT

function usage() {
    cat <<EOF
${0} [-h] -t <thrift version> [-p]
  -h  Print this usage information. Optional.
  -d  Install directory of thrift. Optional. /opt/thrift is the deafault.
  -t  Thrift version. Mandatory. For example '0.12.0'.
  -p  Additional PATH components.
EOF
}

thrift_install_dir="/opt/thrift"
while getopts "ht:p:" OPTION; do
    case ${OPTION} in
        h)
            usage
            exit 0
            ;;
        d)
            thrift_install_dir="${OPTARG}"
            ;;
        p)
            additional_path="${OPTARG}"
            ;;
        t)
            thrift_version="${OPTARG}"
            ;;
        *)
            usage >&2
            exit 1
            ;;
    esac
done

if [[ -z "${thrift_version}" ]]; then
    echo "Thrift version should be defined." >&2
    usage
    exit 1
fi

if [[ -n "${additional_path}" ]]; then
    export PATH="${additional_path}":"${PATH}"
fi

thrift_archive_dir="thrift-${thrift_version}.tar.gz"
thrift_build_dir="/tmp/thrift"
thrift_src_dir="${thrift_build_dir}/thrift"
java_lib_install_dir="${thrift_install_dir}/lib/java"

mkdir --parents "${thrift_src_dir}"
wget --no-verbose \
  "http://xenia.sote.hu/ftp/mirrors/www.apache.org/thrift/"\
"${thrift_version}/${thrift_archive_dir}"                                      \
    --output-document="${thrift_build_dir}/${thrift_archive_dir}"
tar --extract --gunzip --file="${thrift_build_dir}/${thrift_archive_dir}"      \
    --directory="${thrift_src_dir}" --strip-components=1
rm "${thrift_build_dir}/${thrift_archive_dir}"

# Workaround: Maven repository access allowed by https only.
sed --expression='s,http://repo1.maven.org,https://repo1.maven.org,'           \
    --in-place "${thrift_src_dir}/lib/java/gradle.properties"

# TODO gradle proxy definitions.
configure_cmd=("./configure" "--prefix=${thrift_install_dir}"                  \
  "--enable-libtool-lock" "--enable-tutorial=no" "--enable-tests=no"           \
  "--with-libevent" "--with-zlib" "--without-nodejs" "--without-lua"           \
  "--without-ruby" "--without-csharp" "--without-erlang" "--without-perl"      \
  "--without-php" "--without-php_extension" "--without-dart"                   \
  "--without-haskell" "--without-go" "--without-rs" "--without-haxe"           \
  "--without-dotnetcore" "--without-d" "--without-qt4" "--without-qt5"         \
  "--without-python" "--with-java")

pushd "${thrift_src_dir}"

# Configure thrift
"${configure_cmd[@]}"

# Make java jars
pushd "lib/java"
./gradlew assemble
popd

# Make C++ libs
make --jobs="$(nproc)" install

# Install java components by hand
mkdir --parents "${java_lib_install_dir}"
mv "${thrift_src_dir}/lib/java/build/libs/libthrift-${thrift_version}.jar"     \
  "${java_lib_install_dir}"
mv "${thrift_src_dir}/lib/java/build/deps/"*.jar "${java_lib_install_dir}"

popd
