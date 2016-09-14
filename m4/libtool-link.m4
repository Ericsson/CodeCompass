dnl file      : m4/libtool-link.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl
dnl CXX_LIBTOOL_LINK_IFELSE (input, [action-if-true], [action-if-false])
dnl
dnl Similar to AC_LINK_IFELSE except it uses libtool to perform the
dnl linking and it does this using the C++ compiler.
dnl
AC_DEFUN([CXX_LIBTOOL_LINK_IFELSE],[
AC_LANG_SAVE
AC_LANG(C++)

if test -d .libs; then
  delete_libs_dir=no
else
  delete_libs_dir=yes
fi

AC_COMPILE_IFELSE([$1],
[
  ac_try='/usr/bin/env libtool --tag=CXX --mode=link $CXX -no-install $CXXFLAGS $LDFLAGS -o conftest conftest.$OBJEXT $LIBS >&AS_MESSAGE_LOG_FD'
  if _AC_DO_VAR(ac_try); then
    libtool_link_ok=yes
  else
    libtool_link_ok=no
  fi
],
[
  libtool_link_ok=no
])

if test x"$delete_libs_dir" = xyes; then
  rm -rf .libs
fi

if test x"$libtool_link_ok" = xyes; then
[$2]
:
else
[$3]
:
fi

AC_LANG_RESTORE])dnl
