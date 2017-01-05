dnl file      : m4/libodb-mysql.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl LIBODB_MYSQL([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl
AC_DEFUN([LIBODB_MYSQL], [
libodb_mysql_found=no

AC_ARG_WITH(
  [libodb-mysql],
  [AC_HELP_STRING([--with-libodb-mysql=DIR],[location of libodb-mysql build directory])],
  [libodb_mysql_dir=${withval}],
  [libodb_mysql_dir=])

AC_MSG_CHECKING([for libodb-mysql])

# If libodb_mysql_dir was given, add the necessary preprocessor and
# linker flags.
#
if test x"$libodb_mysql_dir" != x; then
  save_CPPFLAGS="$CPPFLAGS"
  save_LDFLAGS="$LDFLAGS"

  AS_SET_CATFILE([abs_libodb_mysql_dir], [$ac_pwd], [$libodb_mysql_dir])

  CPPFLAGS="$CPPFLAGS -I$abs_libodb_mysql_dir"
  LDFLAGS="$LDFLAGS -L$abs_libodb_mysql_dir/odb/mysql"
fi

save_LIBS="$LIBS"
LIBS="-lodb-mysql $LIBS"

CXX_LIBTOOL_LINK_IFELSE([
AC_LANG_SOURCE([
#include <odb/mysql/exceptions.hxx>

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
  catch (const odb::mysql::database_exception& e)
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
[libodb_mysql_found=yes])

if test x"$libodb_mysql_found" = xno; then
  LIBS="$save_LIBS"

  if test x"$libodb_mysql_dir" != x; then
    CPPFLAGS="$save_CPPFLAGS"
    LDFLAGS="$save_LDFLAGS"
  fi
fi

if test x"$libodb_mysql_found" = xyes; then
  AC_MSG_RESULT([yes])
  $1
else
  AC_MSG_RESULT([no])
  $2
fi
])dnl
