dnl file      : m4/libodb-sqlite.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl LIBODB_SQLITE([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl
AC_DEFUN([LIBODB_SQLITE], [
libodb_sqlite_found=no

AC_ARG_WITH(
  [libodb-sqlite],
  [AC_HELP_STRING([--with-libodb-sqlite=DIR],[location of libodb-sqlite build directory])],
  [libodb_sqlite_dir=${withval}],
  [libodb_sqlite_dir=])

AC_MSG_CHECKING([for libodb-sqlite])

# If libodb_sqlite_dir was given, add the necessary preprocessor and
# linker flags.
#
if test x"$libodb_sqlite_dir" != x; then
  save_CPPFLAGS="$CPPFLAGS"
  save_LDFLAGS="$LDFLAGS"

  AS_SET_CATFILE([abs_libodb_sqlite_dir], [$ac_pwd], [$libodb_sqlite_dir])

  CPPFLAGS="$CPPFLAGS -I$abs_libodb_sqlite_dir"
  LDFLAGS="$LDFLAGS -L$abs_libodb_sqlite_dir/odb/sqlite"
fi

save_LIBS="$LIBS"
LIBS="-lodb-sqlite $LIBS"

CXX_LIBTOOL_LINK_IFELSE([
AC_LANG_SOURCE([
#include <odb/sqlite/exceptions.hxx>

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
  catch (const odb::sqlite::database_exception& e)
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
[libodb_sqlite_found=yes])

if test x"$libodb_sqlite_found" = xno; then
  LIBS="$save_LIBS"

  if test x"$libodb_sqlite_dir" != x; then
    CPPFLAGS="$save_CPPFLAGS"
    LDFLAGS="$save_LDFLAGS"
  fi
fi

if test x"$libodb_sqlite_found" = xyes; then
  AC_MSG_RESULT([yes])
  $1
else
  AC_MSG_RESULT([no])
  $2
fi
])dnl
