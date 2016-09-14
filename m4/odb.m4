dnl file      : m4/odb.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl ODB_COMPILER([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN([ODB_COMPILER], [
odb_found=no

AC_ARG_VAR([ODB],[ODB compiler command])
AC_ARG_VAR([ODBFLAGS],[ODB compiler flags])
AC_ARG_VAR([ODBCPPFLAGS],[ODB-specific C++ preprocessor flags])

AC_ARG_WITH(
  [odb],
  [AC_HELP_STRING([--with-odb=DIR],[location of odb compiler build directory])],
  [odb_dir=${withval}],
  [odb_dir=])

AC_MSG_CHECKING([for odb])

save_ODB="$ODB"

if test x"$ODB" = x; then
 if test x"$odb_dir" != x; then
   AS_SET_CATFILE([abs_odb_dir], [$ac_pwd], [$odb_dir])
   ODB="$abs_odb_dir/odb/odb"
 else
   ODB=odb
 fi
fi

$ODB --version 2>/dev/null 1>&2

if test x"$?" = x0; then
  AC_MSG_RESULT([$ODB])
  $1
else
  AC_MSG_RESULT([no])
  ODB="$save_ODB"
  $2
fi

])dnl
