dnl file      : m4/libodb-oracle.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl LIBODB_ORACLE([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl
AC_DEFUN([LIBODB_ORACLE], [
libodb_oracle_found=no

AC_ARG_WITH(
  [libodb-oracle],
  [AC_HELP_STRING([--with-libodb-oracle=DIR],[location of libodb-oracle build directory])],
  [libodb_oracle_dir=${withval}],
  [libodb_oracle_dir=])

AC_MSG_CHECKING([for libodb-oracle])

# If libodb_oracle_dir was given, add the necessary preprocessor and
# linker flags.
#
if test x"$libodb_oracle_dir" != x; then
  save_CPPFLAGS="$CPPFLAGS"
  save_LDFLAGS="$LDFLAGS"

  AS_SET_CATFILE([abs_libodb_oracle_dir], [$ac_pwd], [$libodb_oracle_dir])

  CPPFLAGS="$CPPFLAGS -I$abs_libodb_oracle_dir"
  LDFLAGS="$LDFLAGS -L$abs_libodb_oracle_dir/odb/oracle"
fi

save_LIBS="$LIBS"
LIBS="-lodb-oracle $LIBS"

CXX_LIBTOOL_LINK_IFELSE([
AC_LANG_SOURCE([
#include <odb/oracle/exceptions.hxx>

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
  catch (const odb::oracle::database_exception& e)
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
[libodb_oracle_found=yes])

if test x"$libodb_oracle_found" = xno; then
  LIBS="$save_LIBS"

  if test x"$libodb_oracle_dir" != x; then
    CPPFLAGS="$save_CPPFLAGS"
    LDFLAGS="$save_LDFLAGS"
  fi
fi

if test x"$libodb_oracle_found" = xyes; then
  AC_MSG_RESULT([yes])
  $1
else
  AC_MSG_RESULT([no])
  $2
fi
])dnl
