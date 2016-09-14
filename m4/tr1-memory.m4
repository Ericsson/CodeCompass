dnl file      : m4/tr1-memory.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl TR1_MEMORY
dnl
dnl Check for TR1 <memory> availability. If successful, define HAVE_TR1_MEMORY
dnl as both a macro and conditional as well as set the tr1_memory variable
dnl to 'yes'.
dnl
AC_DEFUN([TR1_MEMORY],
[
tr1_memory=no

AC_MSG_CHECKING([for TR1 <memory>])

CXX_LIBTOOL_LINK_IFELSE([
AC_LANG_SOURCE([
#include <odb/tr1/memory.hxx>

int
main ()
{
  std::tr1::shared_ptr<int> p (new int (10));
  *p = 11;
}
])],
[tr1_memory=yes])

if test x"$tr1_memory" = xyes; then
  AC_MSG_RESULT([yes])
  AC_DEFINE([HAVE_TR1_MEMORY], [1], [Have TR1 <memory>.])
else
  AC_MSG_RESULT([no])
fi

AM_CONDITIONAL([HAVE_TR1_MEMORY], [test x$tr1_memory = xyes])

])dnl
