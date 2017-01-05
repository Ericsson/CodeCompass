#!/usr/bin/env bash

# This script creates a binary package after a Jenkins build.
#
# Variables from jenkins:
#   Database : database type (pgsql, sqlite)
#   PackageName : name of packege (eg.: CodeCompass-brubeck-sqlite )
#   DepsDirectory: path to deps directory

rm -rf "./pkg_${Database}" &>/dev/null;
mkdir -p "./pkg_${Database}/CodeCompass" &>/dev/null;

# Required programs from bin dir
readonly ccBldReqBinProg="
  createdb ctags dropdb initdb pg_basebackup pg_config pg_controldata pg_ctl
  pg_dump pg_dumpall pg_isready pg_receivexlog pg_resetxlog pg_restore
  postgres psql java sqlite3 python python2 python2.7";
# Required lib directories (copy and filter)
readonly ccBldReqLibDirs="lib lib32 lib64"
# Required other directories (copy)
readonly ccBldReqDirs="etc share ssl"
# Install prefix
readonly ccBldInstPrefix=$(readlink -f "./inst_${Database}");
# Temporary package prefix
readonly ccBldPkgPrefix=$(readlink -f "./pkg_${Database}/CodeCompass");

# Set package name if not defined
if [ -z "$PackageName" ]; then
  PackageName="CodeCompass-${Database}-${BuildType}"
fi

# Copyes the given directories with a find filter
#
# param 1: the filter
# param 2: the parent dir of the given directories
# param 3..n: the directories to copy
function copyDirsWithFilter {
  local filter=$1;
  local pdir=$2
  shift 2;

  echo "Copying directories from ${pdir}..."
  pushd $pdir &>/dev/null
  for ddir in $@; do
    if ! [ -d "$ddir" ]; then
      echo "Skipping dir: $ddir"
      continue;
    fi

    echo "Copying ${ddir}... $filter";
    find ${ddir} \
      \( $filter -type d -exec mkdir -p "${ccBldPkgPrefix}/{}" \; \) -or \
      \( $filter -exec cp -f "{}" "${ccBldPkgPrefix}/{}" \; \);

  done
  popd &>/dev/null
}

cp "./scripts/runtime.sh" "$ccBldPkgPrefix/runtime.sh" || exit 1;

# Check dirs
for ddir in "$ccBldInstPrefix" "$DepsDirectory"; do
  if ! [ -d "$ddir" ]; then
    echo "$ddir not found!"
    exit 1;
  fi
done

# Copy CodeCompass
copyDirsWithFilter "-not -name *.a" ${ccBldInstPrefix} "lib bin share usr logger";

# Copy stuff from bin
echo "Copying bin..."
for ffile in $ccBldReqBinProg; do
  cp -f "${DepsDirectory}/bin/${ffile}" "${ccBldPkgPrefix}/bin/${ffile}" \
    || exit 3;
done

# Copy libs
copyDirsWithFilter \
  "-not -name *.a -not -name libclang.so -not -name libLTO.so" \
  ${DepsDirectory} ${ccBldReqLibDirs};
# Copy other stuff
copyDirsWithFilter "" ${DepsDirectory} ${ccBldReqDirs};

# Delete unnecessary files
rm "${ccBldPkgPrefix}/bin/ldlogger";

# Create package
echo "Creating package...";
pushd "${ccBldPkgPrefix}/.." &>/dev/null;
  echo "REVISION: $GIT_COMMIT   TAG: $BUILD_TAG" > CodeCompass/BUILD_INFO;
  tar -jcf "../${PackageName}.tar.bz2" * || exit 5
popd &>/dev/null;
echo "Done!";

rm -rf ${ccBldPkgPrefix} &>/dev/null;
exit 0;
