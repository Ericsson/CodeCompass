dnl file      : m4/libodb.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl LIBODB([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl
AC_DEFUN([LIBODB], [
libodb_found=no

AC_ARG_WITH(
  [libodb],
  [AC_HELP_STRING([--with-libodb=DIR],[location of libodb build directory])],
  [libodb_dir=${withval}],
  [libodb_dir=])

AC_MSG_CHECKING([for libodb])

# If libodb_dir was given, add the necessary preprocessor and linker flags.
#
if test x"$libodb_dir" != x; then
  save_CPPFLAGS="$CPPFLAGS"
  save_LDFLAGS="$LDFLAGS"

  AS_SET_CATFILE([abs_libodb_dir], [$ac_pwd], [$libodb_dir])

  CPPFLAGS="$CPPFLAGS -I$abs_libodb_dir"
  LDFLAGS="$LDFLAGS -L$abs_libodb_dir/odb"
fi

save_LIBS="$LIBS"
LIBS="-lodb $LIBS"

CXX_LIBTOOL_LINK_IFELSE([
AC_LANG_SOURCE([
#include <odb/exception.hxx>

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
  catch (const odb::exception& e)
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
[libodb_found=yes])

if test x"$libodb_found" = xno; then
  LIBS="$save_LIBS"

  if test x"$libodb_dir" != x; then
    CPPFLAGS="$save_CPPFLAGS"
    LDFLAGS="$save_LDFLAGS"
  fi
fi

if test x"$libodb_found" = xyes; then
  AC_MSG_RESULT([yes])
  $1
else
  AC_MSG_RESULT([no])
  $2
fi
])dnl
