dnl file      : m4/libodb-mssql.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl LIBODB_MSSQL([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl
AC_DEFUN([LIBODB_MSSQL], [
libodb_mssql_found=no

AC_ARG_WITH(
  [libodb-mssql],
  [AC_HELP_STRING([--with-libodb-mssql=DIR],[location of libodb-mssql build directory])],
  [libodb_mssql_dir=${withval}],
  [libodb_mssql_dir=])

AC_MSG_CHECKING([for libodb-mssql])

# If libodb_mssql_dir was given, add the necessary preprocessor and
# linker flags.
#
if test x"$libodb_mssql_dir" != x; then
  save_CPPFLAGS="$CPPFLAGS"
  save_LDFLAGS="$LDFLAGS"

  AS_SET_CATFILE([abs_libodb_mssql_dir], [$ac_pwd], [$libodb_mssql_dir])

  CPPFLAGS="$CPPFLAGS -I$abs_libodb_mssql_dir"
  LDFLAGS="$LDFLAGS -L$abs_libodb_mssql_dir/odb/mssql"
fi

save_LIBS="$LIBS"
LIBS="-lodb-mssql $LIBS"

CXX_LIBTOOL_LINK_IFELSE([
AC_LANG_SOURCE([
#include <odb/mssql/exceptions.hxx>

void
f ()
{
}

const char*
g ()
{
  try
  {
    f ();
  }
  catch (const odb::mssql::database_exception& e)
  {
    return e.what ();
  }
  return 0;
}

int
main ()
{
  const char* m (g ());
  return m != 0;
}
])],
[libodb_mssql_found=yes])

if test x"$libodb_mssql_found" = xno; then
  LIBS="$save_LIBS"

  if test x"$libodb_mssql_dir" != x; then
    CPPFLAGS="$save_CPPFLAGS"
    LDFLAGS="$save_LDFLAGS"
  fi
fi

if test x"$libodb_mssql_found" = xyes; then
  AC_MSG_RESULT([yes])
  $1
else
  AC_MSG_RESULT([no])
  $2
fi
])dnl
