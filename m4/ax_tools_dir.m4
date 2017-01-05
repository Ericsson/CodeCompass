AC_DEFUN([AX_TOOLS_DIR],
[
  AC_ARG_WITH([tools],
    [AS_HELP_STRING([--with-tools=@<:@ARG@:>@],
                  [use the tools directory to find dependencies]
    )],
    [
    if test "$withval" = "no"; then
        WANT_TOOLS="no"
    elif test "$withval" = "yes"; then
        WANT_TOOLS="yes"
        ac_tools_path="llvm-config"
    else
        WANT_TOOLS="yes"
        ac_tools_path="$withval"
    fi
    ],
    [WANT_TOOLS="no"]
  )

  if test "x$WANT_TOOLS" = "xyes"; then
      with_llvm_config="$ac_tools_path/llvm-3.2/Release+Asserts/bin/llvm-config"
      with_sqlite="$ac_tools_path/sqlite-autoconf-3071000"
      enable_gtest="$ac_tools_path/gtest-1.7.0"
  fi

])
