dnl file      : m4/libodb-boost.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl LIBODB_BOOST([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl
AC_DEFUN([LIBODB_BOOST], [
libodb_boost_found=no

AC_ARG_WITH(
  [libodb-boost],
  [AC_HELP_STRING([--with-libodb-boost=DIR],[location of libodb-boost build directory])],
  [libodb_boost_dir=${withval}],
  [libodb_boost_dir=])

AC_MSG_CHECKING([for libodb-boost])

# If libodb_boost_dir was given, add the necessary preprocessor and
# linker flags.
#
if test x"$libodb_boost_dir" != x; then
  save_CPPFLAGS="$CPPFLAGS"
  save_LDFLAGS="$LDFLAGS"

  AS_SET_CATFILE([abs_libodb_boost_dir], [$ac_pwd], [$libodb_boost_dir])

  CPPFLAGS="$CPPFLAGS -I$abs_libodb_boost_dir"
  LDFLAGS="$LDFLAGS -L$abs_libodb_boost_dir/odb/boost"
fi

save_LIBS="$LIBS"
LIBS="-lodb-boost $LIBS"

CXX_LIBTOOL_LINK_IFELSE([
AC_LANG_SOURCE([
#include <odb/boost/exception.hxx>

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
  catch (const odb::boost::exception& e)
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
[libodb_boost_found=yes])

if test x"$libodb_boost_found" = xno; then
  LIBS="$save_LIBS"

  if test x"$libodb_boost_dir" != x; then
    CPPFLAGS="$save_CPPFLAGS"
    LDFLAGS="$save_LDFLAGS"
  fi
fi

if test x"$libodb_boost_found" = xyes; then
  AC_MSG_RESULT([yes])
  $1
else
  AC_MSG_RESULT([no])
  $2
fi
])dnl
