dnl file      : m4/pgsql.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl PGSQL
dnl
AC_DEFUN([PGSQL], [

# Client.
#
AC_MSG_CHECKING([for pgsql client program])
AC_ARG_WITH(
  [pgsql-client],
  [AC_HELP_STRING([--with-pgsql-client=PATH], [PostgreSQL client program path (psql by default)])],
  [case $withval in
     yes)
       pgsql_client=psql
       ;;
     no)
       AC_MSG_RESULT([])
       AC_MSG_ERROR([need pgsql client to run the tests])
       ;;
     *)
       pgsql_client=$withval
       ;;
   esac],
  [pgsql_client=psql])

$pgsql_client --version 2>/dev/null 1>&2

if test x"$?" = x0; then
  AC_MSG_RESULT([$pgsql_client])
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([pgsql client is not found; consider using --with-pgsql-client=PATH])
fi

# User.
#
AC_MSG_CHECKING([for pgsql database user])
AC_ARG_WITH(
  [pgsql-user],
  [AC_HELP_STRING([--with-pgsql-user=NAME], [PostgreSQL database user (odb_test by default). The user must be able to login without specifying a password.])],
  [case $withval in
     yes)
       pgsql_user=odb_test
       pgsql_user_set=yes
       ;;
     no)
       pgsql_user_set=no
       ;;
     *)
       pgsql_user=$withval
       pgsql_user_set=yes
       ;;
   esac],
  [pgsql_user=odb_test
   pgsql_user_set=yes])

if test x$pgsql_user_set = xyes; then
  AC_MSG_RESULT(['$pgsql_user'])
else
  AC_MSG_RESULT([none])
fi

# Database name.
#
AC_MSG_CHECKING([for pgsql database name])
AC_ARG_WITH(
  [pgsql-db],
  [AC_HELP_STRING([--with-pgsql-db=NAME], [PostgreSQL database name (odb_test by default). Note that all data in this database WILL BE LOST!])],
  [case $withval in
     yes)
       pgsql_db=odb_test
       pgsql_db_set=yes
       ;;
     no)
       pgsql_db_set=no
       ;;
     *)
       pgsql_db=$withval
       pgsql_db_set=yes
       ;;
   esac],
  [pgsql_db=odb_test
   pgsql_db_set=yes])

if test x$pgsql_db_set = xyes; then
  AC_MSG_RESULT(['$pgsql_db'])
else
  AC_MSG_RESULT([none])
fi

# Host.
#
AC_MSG_CHECKING([for pgsql database host])
AC_ARG_WITH(
  [pgsql-host],
  [AC_HELP_STRING([--with-pgsql-host=HOST], [PostgreSQL database host (localhost by default)])],
  [case $withval in
     yes)
       pgsql_host=
       pgsql_host_set=yes
       ;;
     no)
       pgsql_host_set=no
       ;;
     *)
       pgsql_host=$withval
       pgsql_host_set=yes
       ;;
   esac],
  [pgsql_host_set=no])

if test x$pgsql_host_set = xyes; then
  AC_MSG_RESULT(['$pgsql_host'])
else
  AC_MSG_RESULT([localhost])
fi

# Port.
#
AC_MSG_CHECKING([for pgsql database port])
AC_ARG_WITH(
  [pgsql-port],
  [AC_HELP_STRING([--with-pgsql-port=PORT], [PostgreSQL database port (standard PostgreSQL port by default)])],
  [case $withval in
     yes)
       pgsql_port=0
       pgsql_port_set=yes
       ;;
     no)
       pgsql_port_set=no
       ;;
     *)
       pgsql_port=$withval
       pgsql_port_set=yes
       ;;
   esac],
  [pgsql_port_set=no])

if test x$pgsql_port_set = xyes; then
  AC_MSG_RESULT(['$pgsql_port'])
else
  AC_MSG_RESULT([default])
fi

# Create options file.
#
AC_CONFIG_COMMANDS([pgsql.options],
 [
   rm -f db.options
   echo '#! /bin/sh' >db-driver

   echo 'opt=' >>db-driver

   if test x$pgsql_user_set = xyes; then
     echo "--username '$pgsql_user'" >>db.options
     echo 'opt="$opt --username='"$pgsql_user"'"' >>db-driver
   fi

   if test x$pgsql_db_set = xyes; then
     echo "--dbname '$pgsql_db'" >>db.options
     echo 'opt="$opt --dbname='"$pgsql_db"'"' >>db-driver
   fi

   if test x$pgsql_host_set = xyes; then
     echo "--host '$pgsql_host'" >>db.options
     echo 'opt="$opt --host='"$pgsql_host"'"' >>db-driver
   fi

   if test x$pgsql_port_set = xyes; then
     echo "--port '$pgsql_port'" >>db.options
     echo 'opt="$opt --port='"$pgsql_port"'"' >>db-driver
   fi

   echo 'opt="$opt --quiet"' >>db-driver
   echo 'PGOPTIONS=--client-min-messages=warning' >>db-driver
   echo 'export PGOPTIONS' >>db-driver

   echo 'if test x$[]1 != x; then' >>db-driver
   echo "  exec $pgsql_client "'$opt --set ON_ERROR_STOP=1 -f $[]1' >>db-driver
   echo "else" >>db-driver
   echo "  exec $pgsql_client "'$opt' >>db-driver
   echo "fi" >>db-driver

   chmod +x db-driver
 ],
 [
  pgsql_client="$pgsql_client"

  pgsql_user="$pgsql_user"
  pgsql_user_set="$pgsql_user_set"

  pgsql_db="$pgsql_db"
  pgsql_db_set="$pgsql_db_set"

  pgsql_host="$pgsql_host"
  pgsql_host_set="$pgsql_host_set"

  pgsql_port="$pgsql_port"
  pgsql_port_set="$pgsql_port_set"
 ])

])dnl
