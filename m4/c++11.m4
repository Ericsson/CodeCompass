dnl file      : m4/cxx11.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl CXX11(MACRO, DESCRIPTION)
dnl
dnl Check if we are compiling in the C++11 mode. If we are, define MACRO as
dnl both a macro and conditional as well as set the cxx11 variable to 'yes'.
dnl
AC_DEFUN([CXX11],
[
cxx11=no

AC_MSG_CHECKING([whether we are in C++11 mode])

CXX_LIBTOOL_LINK_IFELSE([
AC_LANG_SOURCE([
int
main ()
{
  int a = 42;

  auto f = [[]](int inp)
  {
    return 2 * inp;
  };

  return f(a);
}
])],
[cxx11=yes])

if test x"$cxx11" = xyes; then
  AC_MSG_RESULT([yes])
  AC_DEFINE([$1], [1], [$2])
else
  AC_MSG_RESULT([no])
fi

AM_CONDITIONAL([$1], [test x$cxx11 = xyes])

])dnl
