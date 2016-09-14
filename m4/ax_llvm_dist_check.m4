AC_DEFUN([AX_LLVM_DIST_CHECK],
[
  AC_ARG_WITH([llvm-config],
    [AS_HELP_STRING([--with-llvm-config=@<:@ARG@:>@],
                  [use LLVM parser @<:@default=yes@:>@, optionally specify the prefix for LLVM library]
    )],
    [
    if test "$withval" = "no"; then
        WANT_LLVM="no"
    elif test "$withval" = "yes"; then
        WANT_LLVM="yes"
        ac_llvm_path="llvm-config"
    else
        WANT_LLVM="yes"
        ac_llvm_path="$withval"
    fi
    ],
    [WANT_LLVM="yes"; ac_llvm_path="llvm-config"]
  )

  LLVM_CPPFLAGS=""
  LLVM_LIBS=""
  LLVM_LDFLAGS=""
  LLVM_VERSION=""

  if test "x$WANT_LLVM" = "xyes"; then
      AC_MSG_CHECKING([for LLVM library])
      ac_llvm_version=$($ac_llvm_path --version)
      if test $? -eq 0; then
        AC_MSG_RESULT([ok])

        ac_llvm_src_root="$($ac_llvm_path --src-root)"
        ac_llvm_obj_root="$($ac_llvm_path --obj-root)"
        ac_llvm_cppflags="$($ac_llvm_path --cppflags)"
        #ac_llvm_cppflags="$ac_llvm_cppflags -I${ac_llvm_obj_root}/tools/clang/include"
        #ac_llvm_cppflags="$ac_llvm_cppflags -I${ac_llvm_src_root}/tools/clang/include"

        LLVM_VERSION="$ac_llvm_version"
        LLVM_LDFLAGS="$($ac_llvm_path --ldflags)"
        LLVM_CPPFLAGS="$ac_llvm_cppflags"
        LLVM_LIBS="$($ac_llvm_path --libs)"


        AC_SUBST(LLVM_CPPFLAGS)
        AC_SUBST(LLVM_LDFLAGS)
        AC_SUBST(LLVM_LIBS)
        AC_SUBST(LLVM_VERSION)
        AC_DEFINE([HAVE_LLVM], [1], [Have the LLVM library])
      else
        AC_MSG_RESULT([not found])
      fi
  fi
])
