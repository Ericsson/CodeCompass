#!/bin/bash

#--- Initial settings ---#

export CC_LOGGER_FILE=~/log_file.log
export CC_LOGGER_GCC_LIKE=g++:gcc:clang
export CC_LOGGER_JAVAC_LIKE=javac

PATH=`echo $PATH | sed -e s/[^:]*://`
THISFILE="$0"
COMPILER=`which $(basename "$THISFILE")`
PATH_FROM_ENVIRONMENT_NEEDED=true

#--- Helper functions ---#

function contains
{
  return $(echo "$1" | grep -q "$2")
}

function isPrefix
{
  return $(echo "$1" | grep -qE "^$2")
}

function getPathsFromEnvironment
{
  if [ $tool != "javac" ] ; then
    local sourceType="c++"
    if [ $tool = "gcc" ] ; then
      sourceType="c"
    fi

    local inside=false
    local isBuiltin;

    echo "" | cpp -x $sourceType -Wp,-v 2>&1 |\
    while read line ; do
      if [ "$line" = "End of search list." ] ; then
        break
      fi

      if $inside ; then
        # We have to skip builtin gcc headers, we only need the paths to the stdlib.
        line=`readlink -f $line`
        isBuiltin=$(echo "$line" | sed -e "s/.*lib\/gcc\/[^\/]*\/[0-9]\.[0-9]\.[0-9]\/.*/BUILTIN/g")
        if [ "$isBuiltin" != "BUILTIN" ]; then
          echo -n "-I$line "
        fi
      fi

      if [ "$line" = "#include <...> search starts here:" ] ; then
        inside=true
      fi
    done
  fi
}

function convertToAbsolute
{
  for var in "$@" ; do
    if (isPrefix "$var" "-I" || isPrefix "$var" "-L") && [ "$var" != "-I" -a "$var" != "-L" ] ; then
      echo -e -n "${var:0:2}$(readlink -f "${var:2}") "
    else
      if [ -a "$var" ]; then
        echo -e -n "$(readlink -f "${var}") "
      else
        echo -e -n "$var "
      fi
    fi
  done
}

#--- Beginning of the script ---#

if contains $(basename $0) "g++" ; then
  tool="g++"
elif contains $(basename $0) "gcc" ; then
  tool="gcc"
elif contains $(basename $0) "javac" ; then
  tool="javac"
else
  tool="unknown"
fi

$COMPILER "$@"

if $PATH_FROM_ENVIRONMENT_NEEDED ; then
  compileParameters="$(getPathsFromEnvironment) $@"
else
  compileParameters="$@"
fi

compileParameters="$(convertToAbsolute $compileParameters)"

#--- Writing output ---#

function printLogToOutput
{
  $CODECOMPASS_HOME/logger/bin/ldlogger $COMPILER $compileParameters
}

if [ -z $sourceFile ] ; then
  printLogToOutput
else
  compileParameters=${compileParameters%@*}
  while read line ; do
    printLogToOutput $line
  done < $sourcesFile
fi
