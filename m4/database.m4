dnl file      : m4/database.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl DATABASE
dnl
AC_DEFUN([DATABASE], [
database=none

AC_MSG_CHECKING([for database to use])

AC_ARG_WITH(
  [database],
  [AC_HELP_STRING([--with-database=DB],
                  [database to use for tests; valid values are: 'mysql', 'sqlite', 'pgsql', 'oracle', and 'mssql'])],
  [case $withval in
     no | yes)
       AC_MSG_RESULT([])
       AC_MSG_ERROR([no database specified in the --with-database option])
       ;;
     mysql)
       database=mysql
       AC_DEFINE([DATABASE_MYSQL], [1], [Using MySQL.])
       DATABASE_CPPFLAGS="-DDATABASE_MYSQL"
       ;;
     sqlite)
       database=sqlite
       AC_DEFINE([DATABASE_SQLITE], [1], [Using SQLite.])
       DATABASE_CPPFLAGS="-DDATABASE_SQLITE"
       ;;
     pgsql)
       database=pgsql
       AC_DEFINE([DATABASE_PGSQL], [1], [Using PostgreSQL.])
       DATABASE_CPPFLAGS="-DDATABASE_PGSQL"
       ;;
     oracle)
       database=oracle
       AC_DEFINE([DATABASE_ORACLE], [1], [Using Oracle.])
       DATABASE_CPPFLAGS="-DDATABASE_ORACLE"
       ;;
     mssql)
       database=mssql
       AC_DEFINE([DATABASE_MSSQL], [1], [Using SQL Server.])
       DATABASE_CPPFLAGS="-DDATABASE_MSSQL"
       ;;
     *)
       AC_MSG_RESULT([])
       AC_MSG_ERROR([unknown database $withval])
       ;;
   esac],
  [
   AC_MSG_RESULT([])
   AC_MSG_ERROR([no database specified with the --with-database option])
  ])


AC_MSG_RESULT([$database])

dnl default-enabled index creation can be turned off
AC_ARG_ENABLE([indexes],
  AS_HELP_STRING([--disable-indexes], [Disable database index creation]),
  [case "${enableval}" in
    yes)
      AC_MSG_RESULT([Database index generation is enabled])
      ;;
    no)
      AC_DEFINE([NO_INDICES], [1], [Database index generation is disabled.])
      AC_MSG_RESULT([Database index generation is disabled])
      DATABASE_CPPFLAGS="${DATABASE_CPPFLAGS} -DNO_INDICES"
      ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --disable-indexes]) ;;
  esac],[])


AC_SUBST([database])
AC_SUBST([DATABASE_CPPFLAGS])

AM_CONDITIONAL([DATABASE_MYSQL], [test x$database = xmysql])
AM_CONDITIONAL([DATABASE_SQLITE], [test x$database = xsqlite])
AM_CONDITIONAL([DATABASE_PGSQL], [test x$database = xpgsql])
AM_CONDITIONAL([DATABASE_ORACLE], [test x$database = xoracle])
AM_CONDITIONAL([DATABASE_MSSQL], [test x$database = xmssql])

])dnl
