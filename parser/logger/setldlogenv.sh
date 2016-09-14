#!/bin/echo Do not run this script! Use source

if [ "$LDLOGGER_HOME" == "" ]; then
  LDLOGGER_HOME="../logger"
fi

# Prints usage
function cclenv_print_usage {
  echo "Usage:";
  echo "source ./setldlogenv.sh /log/file/path";
}

# Finds the ldlogger.so
#
# If the library found then sets the CC_LOGGER_LIBPATH variable.
function cclenv_find_logger {
  local ccRoot=$LDLOGGER_HOME

  CC_LOGGER_LIBPATH=$(readlink -f "${ccRoot}/lib");
  CC_LOGGER_LIBPATH_64=$(readlink -f "${CC_LOGGER_LIBPATH}/x86_64/ldlogger.so");
  CC_LOGGER_LIBPATH_32=$(readlink -f "${CC_LOGGER_LIBPATH}/i386/ldlogger.so");
  if ( ! [ -a "$CC_LOGGER_LIBPATH_64" ] ) && ( ! [ -a "$CC_LOGGER_LIBPATH_32" ] ); then
    echo "Logger library not found in ${CC_LOGGER_LIBPATH}!"
    return 1;
  fi

  return 0;
}

function cclenv_main {
  local logDefaultDirs="yes";

  #--- The script starts here ---#
  if [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
    cclenv_print_usage;
    return 0;
  elif [ $# -ne 1 ]; then
    cclenv_print_usage;
    return -1;
  fi

  if [ -n "${CC_LOGGER_FILE}" ]; then
    echo "Log environment is already set!"
    return 0;
  fi
  
  #--- Check logger lib ---#
  if ! cclenv_find_logger; then
    return -1;
  fi
  
  #--- Set output file ---#
  CC_LOGGER_FILE=$(readlink -f "${1}");
  if [ -a "${CC_LOGGER_FILE}" ]; then
    echo "Warning: ${CC_LOGGER_FILE} already exists!";
  fi

  touch "${CC_LOGGER_FILE}" || exit -1;
  touch "${CC_LOGGER_FILE}.lock" || exit -1;
  
  #--- GCC like commands ---#
  if [ -z ${CC_LOGGER_GCC_LIKE+x} ]; then
    CC_LOGGER_GCC_LIKE="gcc:g++:clang:clang++:cc:c++";
  fi 
  
  #--- Javac like commands ---#
  if [ -z ${CC_LOGGER_JAVAC_LIKE+x} ]; then
    CC_LOGGER_JAVAC_LIKE="javac";
  fi

  if [ -n "${CC_LOGGER_NO_DEF_DIRS}" ]; then
    logDefaultDirs="no"
  fi
  
  #--- Print parameters ---#
  echo "**********";
  echo "Logger libraries:             ${CC_LOGGER_LIBPATH}";
  echo "Log file:                     ${CC_LOGGER_FILE}";
  echo "Log default dirs:             ${logDefaultDirs}";
  echo "Logged as GCC like (list):    ${CC_LOGGER_GCC_LIKE}"; 
  echo "Logged as JAVAC like (list):  ${CC_LOGGER_JAVAC_LIKE}";
  echo "**********";
  
  #--- Export variables ---#
  export CC_LOGGER_FILE;
  export CC_LOGGER_GCC_LIKE;
  export CC_LOGGER_JAVAC_LIKE;
  
  if [ -n "${LD_PRELOAD}" ]; then
    LD_PRELOAD="ldlogger.so:${LD_PRELOAD}";
  else
    LD_PRELOAD="ldlogger.so";
  fi

  if [ -n "${LD_LIBRARY_PATH}" ]; then
    LD_LIBRARY_PATH="${CC_LOGGER_LIBPATH}:${LD_LIBRARY_PATH}";
  else
    LD_LIBRARY_PATH="${CC_LOGGER_LIBPATH}";
  fi

  export LD_PRELOAD;
  export LD_LIBRARY_PATH;

  return 0;
}

cclenv_main "$@";

