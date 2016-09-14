dnl $Id: graphviz.m4,v 1.5 2006/11/24 13:38:54 raimc Exp $

dnl
dnl Check --with-graphviz[=PREFIX] is specified and graphviz is installed.
dnl
AC_DEFUN([CONFIG_LIB_GRAPHVIZ],
[ AC_PREREQ(2.57)dnl
  AC_ARG_WITH([graphviz],
  AC_HELP_STRING([--with-graphviz=PREFIX],
                 [Use GRAPHVIZ Library]),
              [with_graphviz=$withval],
              [with_graphviz=yes])
  dnl set GRAPHVIZ related variables
  GRAPHVIZ_CPPFLAGS=
  GRAPHVIZ_LDFLAGS=
  GRAPHVIZ_RPATH=
  GRAPHVIZ_LIBS=
  if test $with_graphviz = no
  then
    graphviz_functional=no
  else
  if test $with_graphviz != yes; then
    AC_MSG_NOTICE(checking for Graphviz Library)
    dnl Remove needless trailing slashes because it can confuse tests later.
    with_graphviz=`echo $with_graphviz | sed -e 's,\(.*\)/$,\1,g'`

    dnl check if program dot is in the user-defined graphviz library path
    AC_PATH_PROG([DOT], [dot], [$with_graphviz/bin/dot],
                 [no-dot-found], [$with_graphviz/bin])
  else
    dnl search for program dot from the graphviz library
    AC_PATH_PROG([DOT], [dot])
  fi

  dnl get dot version
  if test -n "$DOT";
  then
    changequote(<<, >>)
    graphviz_include_path=`echo $DOT | sed -e 's,bin/dot,include/graphviz,'`
    graphviz_lib_path=`echo $DOT | sed -e 's,bin/dot,lib/graphviz,'`
    dot_version=`$DOT -V 2>&1 | sed 's/.* \([0-9]*\.[0-9]*\).*/\1/p; d'`
    dot_major_version=`expr $dot_version : '\([0-9]*\)\.[0-9]*'`
    dot_minor_version=`expr $dot_version : '[0-9]*\.\([0-9]*\)'`
    changequote([, ])
  fi

  AC_MSG_NOTICE([found Graphviz Library Version $dot_version])

  if test $dot_major_version -ge 2;
  then
    if test $dot_minor_version -ge 6;
    then
      graphviz_old=0
      graphviz_headers="#include <gvc.h>"
      graphviz_testprg="GVC_t *gvc;gvc=(GVC_t*)gvContext();gvFreeContext(gvc);"
      GRAPHVIZ_LIBS="-lgvc"
    elif test $dot_minor_version -eq 4;
    then
      graphviz_old=1
      graphviz_headers="#include <gvc.h>"
      graphviz_testprg="GVC_t *gvc;gvc=(GVC_t*)gvContext();gvCleanup(gvc);"
      GRAPHVIZ_LIBS="-lgvc"
    elif test $dot_minor_version -lt 4;
    then
      graphviz_old=2
      graphviz_headers="#include <dotneato.h> #include <gvrender.h>"
      graphviz_testprg="GVC_t *gvc;gvc=gvNEWcontext(NULL,NULL);gvFREEcontext(gvc);"
      GRAPHVIZ_LIBS="-ldotneato"
    fi
  else
    AC_MSG_RESULT([installed version of graphviz is too old!])
    AC_MSG_RESULT([disabling graphviz functionality!!!!!!!!!])
  fi

  if test $dot_major_version -ge 2;
  then
    GRAPHVIZ_CPPFLAGS="-I$graphviz_include_path"
    GRAPHVIZ_LDFLAGS="-L$graphviz_lib_path"

    if test "$HOST_TYPE" = "darwin"; then
      GRAPHVIZ_RPATH=
    else
      GRAPHVIZ_RPATH="-Wl,-rpath,$graphviz_lib_path"
    fi

    dnl check if GRAPHVIZ Library is functional
    AC_MSG_CHECKING([correct functioning of GRAPHVIZ])
    AC_LANG_PUSH(C)

    dnl cach values of some global variables
    graphviz_save_CFLAGS="$CFLAGS"
    graphviz_save_CPPFLAGS="$CPPFLAGS"
    graphviz_save_LDFLAGS="$LDFLAGS"
    graphviz_save_LIBS="$LIBS"

    dnl temporarily add GRAPHVIZ specific stuff to global variables
    CFLAGS="$CFLAGS -Wno-unknown-pragmas"
    CPPFLAGS="$CPPFLAGS $GRAPHVIZ_CPPFLAGS"
    LDFLAGS="$LDFLAGS $GRAPHVIZ_LDFLAGS"
    LIBS="$GRAPHVIZ_LIBS $LIBS"

    dnl can we link a mini program with graphviz?
    AC_TRY_LINK([$graphviz_headers],
      [$graphviz_testprg],
      [graphviz_functional=yes],
      [graphviz_functional=no])

    if test $graphviz_functional = yes; then
      AC_MSG_RESULT([$graphviz_functional])
    else
      AC_MSG_RESULT([$graphviz_functional:
                     CPPFLAGS=$CPPFLAGS
                     LDFLAGS=$LDFLAGS
                     LIBS=$LIBS])
      AC_MSG_RESULT([Can not link to GRAPHVIZ Library])
      AC_MSG_RESULT([odeSolver will be installed without Graphviz functionality])
    fi

    dnl reset global variables to cached values
    CFLAGS=$graphviz_save_CFLAGS
    CPPFLAGS=$graphviz_save_CPPFLAGS
    LDFLAGS=$graphviz_save_LDFLAGS
    LIBS=$graphviz_save_LIBS
    AC_LANG_POP(C)

  else
    $graphviz_functional=no
  fi

  fi

  if test $graphviz_functional = yes; then
    AC_DEFINE([USE_GRAPHVIZ], 1, [Define to 1 to use the GRAPHVIZ Library])
    AC_SUBST(USE_GRAPHVIZ, 1)
    AC_DEFINE_UNQUOTED(GRAPHVIZ_MAJOR_VERSION, $dot_major_version,
              [Major Version of GRAPHVIZ Library])
dnl    AC_SUBST(GRAPHVIZ_MAJOR_VERSION,  $dot_major_version)
    AC_DEFINE_UNQUOTED(GRAPHVIZ_MINOR_VERSION, $dot_minor_version,
              [Minor Version of GRAPHVIZ Library])
dnl    AC_SUBST(GRAPHVIZ_MINOR_VERSION, $dot_minor_version)
    AC_SUBST(GRAPHVIZ_CPPFLAGS)
    AC_SUBST(GRAPHVIZ_LDFLAGS)
    AC_SUBST(GRAPHVIZ_RPATH)
    AC_SUBST(GRAPHVIZ_LIBS)
  else
    AC_DEFINE([USE_GRAPHVIZ], 0, [Define to 1 to use the GRAPHVIZ Library])
    AC_SUBST(USE_GRAPHVIZ, 0)
    AC_SUBST(GRAPHVIZ_CPPFLAGS, "")
    AC_SUBST(GRAPHVIZ_LDFLAGS, "")
    AC_SUBST(GRAPHVIZ_RPATH, "")
    AC_SUBST(GRAPHVIZ_LIBS, "")
  fi
])
