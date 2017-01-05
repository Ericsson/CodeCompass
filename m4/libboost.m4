dnl file      : m4/libboost.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl LIBBOOST([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl
AC_DEFUN([LIBBOOST], [
libboost_found=no

AC_MSG_CHECKING([for boost base headers])

AC_ARG_WITH(
  [boost],
  [AC_HELP_STRING([--with-boost=DIR],[location of boost build directory])],
  [libboost_dir=${withval}],
  [libboost_dir=])

# If libboost_dir was given, add the necessary preprocessor and linker flags.
#
if test x"$libboost_dir" != x; then
  save_CPPFLAGS="$CPPFLAGS"
  save_LDFLAGS="$LDFLAGS"

  AS_SET_CATFILE([abs_libboost_dir], [$ac_pwd], [$libboost_dir])

  CPPFLAGS="$CPPFLAGS -I$abs_libboost_dir"
  LDFLAGS="$LDFLAGS -L$abs_libboost_dir/stage/lib"
fi

CXX_LIBTOOL_LINK_IFELSE([
AC_LANG_SOURCE([
#include <boost/version.hpp>

#ifndef BOOST_VERSION
#  error BOOST_VERSION not defined
#endif

int
main ()
{
}
])],
[
libboost_found=yes
])

if test x"$libboost_found" = xno; then
  if test x"$libboost_dir" != x; then
    CPPFLAGS="$save_CPPFLAGS"
    LDFLAGS="$save_LDFLAGS"
  fi
fi

if test x"$libboost_found" = xyes; then
  AC_MSG_RESULT([yes])
  $1
else
  AC_MSG_RESULT([no])
  $2
fi
])dnl
dnl
dnl LIBBOOST_HEADER_LIB(NAME, SOURCE, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
dnl
AC_DEFUN([LIBBOOST_HEADER_LIB], [
libboost_$1_found=no

AC_MSG_CHECKING([for boost $1 library])
CXX_LIBTOOL_LINK_IFELSE([$2],[libboost_$1_found=yes])

if test x"$libboost_$1_found" = xyes; then
  AC_MSG_RESULT([yes])
  [$3]
else
  AC_MSG_RESULT([no])
  [$4]
fi
])dnl
dnl
dnl LIBBOOST_LIB(NAME, SOURCE, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
dnl
AC_DEFUN([LIBBOOST_LIB], [
libboost_$1_found=no

AC_MSG_CHECKING([for boost $1 library])

save_LIBS="$LIBS"
LIBS="-lboost_$1 $LIBS"

CXX_LIBTOOL_LINK_IFELSE([$2],[libboost_$1_found=yes])

# Try to fall back on the -mt version for backwards-compatibility.
#
if test x"$libboost_$1_found" = xno; then
   LIBS="-lboost_$1-mt $save_LIBS"
   CXX_LIBTOOL_LINK_IFELSE([$2],[libboost_$1_found=yes])
fi

if test x"$libboost_$1_found" = xno; then
  LIBS="$save_LIBS"
fi

if test x"$libboost_$1_found" = xyes; then
  AC_MSG_RESULT([yes])
  [$3]
else
  AC_MSG_RESULT([no])
  [$4]
fi
])dnl
dnl
dnl LIBBOOST_SMART_PTR([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN([LIBBOOST_SMART_PTR], [
LIBBOOST_HEADER_LIB([smart_ptr],[
AC_LANG_SOURCE([
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

int
main ()
{
  boost::shared_ptr<int> sp (new int (10));
  boost::weak_ptr<int> wp (sp);
}
])],
[$1],
[$2])
])dnl
dnl
dnl LIBBOOST_UNORDERED([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN([LIBBOOST_UNORDERED], [
LIBBOOST_HEADER_LIB([unordered],[
AC_LANG_SOURCE([
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

int
main ()
{
  boost::unordered_set<int> s;
  boost::unordered_map<int, int> m;

  s.insert (1);
  return m.find (1) != m.end ();
}
])],
[$1],
[$2])
])dnl
dnl
dnl LIBBOOST_SYSTEM([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN([LIBBOOST_SYSTEM], [
LIBBOOST_LIB([system],[
AC_LANG_SOURCE([
int
main ()
{
}
])],
[$1],
[$2])
])dnl
dnl
dnl LIBBOOST_DATE_TIME([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN([LIBBOOST_DATE_TIME], [
LIBBOOST_LIB([date_time],[
AC_LANG_SOURCE([
#include <boost/date_time/gregorian/gregorian.hpp>

int
main ()
{
  boost::gregorian::greg_month m (1);
  const char* s (m.as_short_string ());
  return s == 0;
}
])],
[$1],
[$2])
])dnl
