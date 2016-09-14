##!/bin/echo Do not run this script! Use source

if ( $?LDLOGGER_HOME == 0 ) then
  set LDLOGGER_HOME="../logger"
endif

# Prints usage
alias cclenv_print_usage '\
    echo "Usage:"\
    echo "source ./setldlogenv.csh /log/file/path"\
    '

# Finds the ldlogger.so
#
# If the library found then sets the CC_LOGGER_LIBPATH variable.
alias cclenv_find_logger '\
  set ccRoot=$LDLOGGER_HOME\
  set CC_LOGGER_LIBPATH=`readlink -f "$ccRoot/lib"`\
  set CC_LOGGER_LIBPATH_64=`readlink -f "$CC_LOGGER_LIBPATH/x86_64/ldlogger.so"`\
  set CC_LOGGER_LIBPATH_32=`readlink -f "$CC_LOGGER_LIBPATH/i386/ldlogger.so"`\
'
#  if ( ! (-d "$CC_LOGGER_LIBPATH_64" ) &&  ! (-d "$CC_LOGGER_LIBPATH_32" )) then\
#    echo "Logger library not found in $CC_LOGGER_LIBPATH!"\
#    return 1\
#  endif\
#  return 0\


  set logDefaultDirs="yes"
  #--- The script starts here ---#
  if ( "$1" == "--help"  || "$1" == "-h" ) then
    cclenv_print_usage
    exit 0
  else 
    if ( $# < 1 || $# >1 ) then
	cclenv_print_usage
	exit -1
    endif    
  endif

  if ( $?CC_LOGGER_FILE == 1 ) then
    echo "Log environment is already set!"
    exit 0
  endif
  
  #--- Check logger lib ---#
  cclenv_find_logger
  
  #--- Set output file ---#
  setenv CC_LOGGER_FILE `readlink -f "$1"`
  if ( -e $CC_LOGGER_FILE ) then
    echo "Warning: $CC_LOGGER_FILE already exists!"
  endif

  touch "$CC_LOGGER_FILE"
  touch "$CC_LOGGER_FILE".lock
  
  #--- GCC like commands ---#
  if ( $?CC_LOGGER_GCC_LIKE == 0 ) then
    setenv CC_LOGGER_GCC_LIKE "gcc:g++:clang"
  endif 
  
  #--- Javac like commands ---#
  if ($?CC_LOGGER_JAVAC_LIKE == 0) then
    setenv CC_LOGGER_JAVAC_LIKE "gcc:g++:clang:clang++:cc:c++"
  endif

  if ( $?CC_LOGGER_NO_DEF_DIRS == 1) then
    set logDefaultDirs="no"
  endif
  
  #--- Print parameters ---#
  echo "**********"
  echo "Logger libraries:             $CC_LOGGER_LIBPATH"
  echo "Log file:                     $CC_LOGGER_FILE"
  echo "Log default dirs:             $logDefaultDirs"
  echo "Logged as GCC like (list):    $CC_LOGGER_GCC_LIKE"
  echo "Logged as JAVAC like (list):  $CC_LOGGER_JAVAC_LIKE"
  echo "**********"
  
  if ( $?LD_PRELOAD == 1 ) then
    setenv LD_PRELOAD "ldlogger.so:$LD_PRELOAD"
  else
    setenv LD_PRELOAD "ldlogger.so"
  endif

  if ( $?LD_LIBRARY_PATH == 1 ) then
    setenv LD_LIBRARY_PATH "$CC_LOGGER_LIBPATH":"$LD_LIBRARY_PATH"
  else
    setenv LD_LIBRARY_PATH "$CC_LOGGER_LIBPATH"
  endif

  exit 0


