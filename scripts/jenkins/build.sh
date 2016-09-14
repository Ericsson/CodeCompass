#!/usr/bin/env bash

# This script is for Jenkins build support. Should be used from the
# "$WORKSPACE" directory.
#
# Variables from jenkins:
#   Database      : database type (pgsql, sqlite)
#   BuildType     : release / debug [optional]
#   CpuCount      : CPU count for -j option of make [optional]
#   DepsDirectory : path to deps directory

# CPU count for -j option of make
if [ -z "$CpuCount" ]; then
  ccBldCpuCount=$(grep processor /proc/cpuinfo|wc -l);
else
  ccBldCpuCount=$CpuCount;
fi
# Install prefix
ccBldInstPrefix=$(readlink -f "./inst_${Database}");
# Configure parameters
ccBldConfigParms="--prefix=${ccBldInstPrefix} --enable-pythonps";
# Do a make check? (1 = true, 0 = false)
ccBldDoCheck=0;

# Add database type to configure parameters
case "$Database" in
  "pgsql")
    ccBldConfigParms="$ccBldConfigParms --with-database=pgsql"
    ;;
  "sqlite")
    ccBldConfigParms="$ccBldConfigParms --with-database=sqlite --disable-indexes"
    ccBldDoCheck=1;
    ;;
  *)
    echo "Unknonwn database: '$Database'"
    exit 1
    ;;
esac

export GTEST_OUTPUT="xml"
./autogen.sh || exit 1

if [ "$BuildType" = "debug" ]; then
  CXXFLAGS="-O0 -ggdb3 $CXXFLAGS_EXTRA" CFLAGS="-O0 -ggdb3 $CFLAGS_EXTRA" ./configure $ccBldConfigParms || exit 3
else
  CXXFLAGS="-O2 $CXXFLAGS_EXTRA" CFLAGS="-O2 $CFLAGS_EXTRA" ./configure $ccBldConfigParms || exit 3
fi

make -j${ccBldCpuCount} || exit 5;
make install || exit 7;

if [ $ccBldDoCheck -eq 1 ]; then
  # We need to put the stdc++ to the LD_PRELOAD for the tests
  export LD_PRELOAD="${DepsDirectory}/lib64/libstdc++.so.6";
  make -j${ccBldCpuCount} check || echo "Check failed!";
fi

exit 0;
