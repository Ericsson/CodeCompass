dnl file      : m4/sqlite.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl SQLITE
dnl
AC_DEFUN([SQLITE], [

# Database file.
#
AC_MSG_CHECKING([for sqlite database file])
AC_ARG_WITH(
  [sqlite-db],
  [AC_HELP_STRING([--with-sqlite-db=FILE], [SQLite database file (odb-test.db by default). Note that all data in this database WILL BE LOST!])],
  [case $withval in
     yes)
       sqlite_db=odb-test.db
       sqlite_db_set=yes
       ;;
     no)
       sqlite_db_set=no
       ;;
     *)
       sqlite_db=$withval
       sqlite_db_set=yes
       ;;
   esac],
  [sqlite_db=odb-test.db
   sqlite_db_set=yes])

if test x$sqlite_db_set = xyes; then

  # Make it an absolute path unless it is one of the special values.
  #
  if test x$sqlite_db != x -a x$sqlite_db != x:memory:; then
    AS_SET_CATFILE([abs_sqlite_db], [$ac_pwd], [$sqlite_db])
    sqlite_db=$abs_sqlite_db
  fi

  AC_MSG_RESULT(['$sqlite_db'])
else
  AC_MSG_RESULT([none])
fi

# Create options file.
#
AC_CONFIG_COMMANDS([sqlite.options],
 [
   rm -f db.options

   if test x$sqlite_db_set = xyes; then
     echo "--database '$sqlite_db'" >>db.options
   fi
 ],
 [
  sqlite_db="$sqlite_db"
  sqlite_db_set="$sqlite_db_set"
 ])

])dnl
