#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

/AC_PREREQ([2.50])
AC_INIT([CodeCompass], [5.0], [model@plc.inf.elte.hu])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([model/include/model/workspace.h])
AC_CONFIG_HEADERS([config.h])

LT_INIT([dlopen])
AM_INIT_AUTOMAKE([foreign subdir-objects parallel-tests])

AC_CONFIG_MACRO_DIR([m4])

AX_TOOLS_DIR

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_JAVAC

AC_LANG([C++])


AX_CXX_COMPILE_STDCXX_14
AX_CXXFLAGS_WARN_ALL

DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN

if test "X$DX_DOXYGEN" = "X"; then
  AC_MSG_ERROR([cannot find Doxygen])
fi

#ednikru adding webplugin dir definition
#AC_SUBST([webplugindir], ['${libdir}/webplugins'])

# Detect git rev
if test "X$GIT_REVISION" = "X"; then
  AC_PATH_PROG(gitVerCmd, git)
  if test "X$gitVerCmd" != "X" && ($gitVerCmd rev-parse HEAD &> /dev/null); then
    CC_GIT_VERSION=`$gitVerCmd rev-parse HEAD`
  else
    CC_GIT_VERSION="norev"
  fi

  AC_MSG_CHECKING(git revision)
  AC_MSG_RESULT(${CC_GIT_VERSION})
else
  AC_MSG_CHECKING(git revision)
  CC_GIT_VERSION="$GIT_REVISION"
  AC_MSG_RESULT(${CC_GIT_VERSION} from environment)
fi

AC_DEFINE_UNQUOTED(CC_GIT_VERSION, ["$CC_GIT_VERSION"], [repository git version])
AC_SUBST(CC_GIT_VERSION)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T


# Checks for library functions.
AC_TDD_GCOV

# XERCES
AX_PTHREAD
AX_LIB_XERCES([3.1.0])
AM_CONDITIONAL([HAVE_XERCES], [test "$XERCES_VERSION" != ""])


if test "x$HAVE_XERCES" != "xyes"; then
   AC_MSG_ERROR([cannot find the Apache Xerces library])
fi

CONFIG_LIB_GRAPHVIZ
AS_IF(test "x$USE_GRAPHVIZ" != "x1", 
   AC_MSG_ERROR([cannot find the graphviz library.]))


# ---- Google Test -------------------------------------------------------------
# Google's framework for writing C++ tests on a variety of platforms (Linux, Mac
# OS X, Windows, Cygwin, Windows CE, and Symbian). Based on the xUnit
# architecture. Supports automatic test discovery, a rich set of assertions,
# user-defined assertions, death tests, fatal and non-fatal failures, value- and
# type-parameterized tests, various options for running the tests, and XML test
# report generation.
#
# http://code.google.com/p/googletest/
#
m4_include(m4/gtest.m4)
GTEST_LIB_CHECK(1.7.0,[
TEST_LIBS="$GTEST_LIBS"
TEST_LDFLAGS="$GTEST_LDFLAGS"
TEST_CPPFLAGS="$GTEST_CPPFLAGS"
],true)

AX_LLVM_DIST_CHECK(3.4)

AC_SUBST(TEST_LIBS)
AC_SUBST(TEST_CPPFLAGS)
AC_SUBST(TEST_LDFLAGS)
AC_SUBST(COVERAGE_CXXFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)

CXX11([HAVE_CXX11], [Compiling in the C++11 mode.])
CXX14([HAVE_CXX14], [Compiling in the C++14 mode.])

# Check for LDAP
#
AC_CHECK_HEADER(ldap.h, [],
  AC_MSG_ERROR([LDAP client library header file not found!]))
AC_CHECK_LIB(ldap, ldap_initialize, [],
  AC_MSG_ERROR([LDAP client library not found!]))

# --------------- ODB related --------------
# Check for the ODB compiler.
#
ODB_COMPILER([], [AC_MSG_ERROR([odb compiler is not found; consider setting ODB variable or using --with-odb=DIR])])

# Check for the ODB libs.
#
LIBODB([], [AC_MSG_ERROR([libodb is not found; consider using --with-libodb=DIR])])

# Check for TR1 <memory> availability (has to be after libodb).
#
TR1_MEMORY

# Check for boost.
#
odb_examples_boost=yes
LIBBOOST([], [odb_examples_boost=no])
LIBBOOST_SYSTEM
LIBBOOST_SMART_PTR([], [odb_examples_boost=no])
LIBBOOST_UNORDERED([], [odb_examples_boost=no])
LIBBOOST_DATE_TIME([], [odb_examples_boost=no])

# Check for libodb-boost.
#
LIBODB_BOOST([], [odb_examples_boost=no])

# Check which database we are using.
#
DATABASE
case $database in
  mysql)
    LIBODB_MYSQL([], [AC_MSG_ERROR([libodb-mysql is not found; consider using --with-libodb-mysql=DIR])])
    MYSQL
    ;;
  sqlite)
    LIBODB_SQLITE([], [AC_MSG_ERROR([libodb-sqlite is not found; consider using --with-libodb-sqlite=DIR])])
    SQLITE
    ;;
  pgsql)
    LIBODB_PGSQL([], [AC_MSG_ERROR([libodb-pgsql is not found; consider using --with-libodb-pgsql=DIR])])
    PGSQL
    ;;
  oracle)
    LIBODB_ORACLE([], [AC_MSG_ERROR([libodb-oracle is not found; consider using --with-libodb-oracle=DIR])])
    ORACLE
    ;;
  mssql)
    LIBODB_MSSQL([], [AC_MSG_ERROR([libodb-mssql is not found; consider using --with-libodb-mssql=DIR])])
    MSSQL
    ;;
esac

#############################
# Statistics server         #
#############################

dnl default-disabled statistics server
AC_ARG_ENABLE([uscs],
	AS_HELP_STRING([--enable-uscs], [Enable building Usage Statistic Collector Server]),
	[case "${enableval}" in
	  yes) uscs=true ;;
	  no)  uscs=false ;;
	  *) AC_MSG_ERROR([bad value ${enableval} for --enable-uscs]) ;;
	esac],[uscs=false])

AM_CONDITIONAL([ENABLE_USCS], [test x$uscs = xtrue])
AM_COND_IF([ENABLE_USCS], [AC_DEFINE(ENABLE_USCS,[1],[Enable building Usage Statistic CollectorServer])])

if test "x$CC_USCS_DEFAULT_HOST" != "x"; then
  AC_DEFINE_UNQUOTED([CC_USCS_DEFAULT_HOST], ["$CC_USCS_DEFAULT_HOST"], ["Default USCS host"])
fi
if test "x$CC_USCS_DEFAULT_PORT" != "x"; then
  AC_DEFINE_UNQUOTED([CC_USCS_DEFAULT_PORT], [$CC_USCS_DEFAULT_PORT], ["Default USCS port"])
fi

#############################
# Python parser and service #
#############################

AC_ARG_ENABLE([pythonps],
	AS_HELP_STRING([--enable-pythonps], [Enable python parser]),
	[case "${enableval}" in
	  yes) pythonps=true ;;
	  no)  pythonps=false ;;
	  *) AC_MSG_ERROR([bad value ${enableval} for --enable-pythonps]) ;;
	esac],[pythonps=false])

AM_CONDITIONAL([ENABLE_PYTHON_PARSER], [test x$pythonps = xtrue])
AM_COND_IF([ENABLE_PYTHON_PARSER], [AC_DEFINE(ENABLE_PYTHON_PARSER,[1],[Enable python parser])])

#############################
# Other stuff               #
#############################

COMMON_LIBS="$LIBS"
LIBS=""
AC_SUBST(COMMON_LIBS)

# env.sh guard
# if it is configure with env.sh loaded, it must be built with the same.
AM_CONDITIONAL([ENV_SH_USED], [test x"$grocker_env_set" != x])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
