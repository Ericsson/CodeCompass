dnl file      : m4/mssql.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl MSSQL
dnl
AC_DEFUN([MSSQL], [

# Client.
#
AC_MSG_CHECKING([for mssql client program])
AC_ARG_WITH(
  [mssql-client],
  [AC_HELP_STRING([--with-mssql-client=PATH], [SQL Server client program path (sqlcmd by default)])],
  [case $withval in
     yes)
       mssql_client=sqlcmd
       ;;
     no)
       AC_MSG_RESULT([])
       AC_MSG_ERROR([need mssql client to run the tests])
       ;;
     *)
       mssql_client=$withval
       ;;
   esac],
  [mssql_client=sqlcmd])

$mssql_client -? 2>/dev/null 1>&2

if test x"$?" = x0; then
  AC_MSG_RESULT([$mssql_client])
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([mssql client is not found; consider using --with-mssql-client=PATH])
fi

# User. If 'no' is specified, then use Windows authentication.
#
AC_MSG_CHECKING([for mssql database user])
AC_ARG_WITH(
  [mssql-user],
  [AC_HELP_STRING([--with-mssql-user=NAME], [SQL Server database user (odb_test by default)])],
  [case $withval in
     yes)
       mssql_user=odb_test
       mssql_user_set=yes
       ;;
     no)
       mssql_user_set=no
       ;;
     *)
       mssql_user=$withval
       mssql_user_set=yes
       ;;
   esac],
  [mssql_user=odb_test
   mssql_user_set=yes])

if test x$mssql_user_set = xyes; then
  AC_MSG_RESULT(['$mssql_user'])
else
  AC_MSG_RESULT([none])
fi

# Password. Can be left unspecified if using Windows authentication.
#
AC_MSG_CHECKING([for mssql database password])
AC_ARG_WITH(
  [mssql-password],
  [AC_HELP_STRING([--with-mssql-password=PASS], [SQL Server database password (odb_test by default)])],
  [case $withval in
     yes)
       mssql_password=odb_test
       mssql_password_set=yes
       ;;
     no)
       mssql_password_set=no
       ;;
     *)
       mssql_password=$withval
       mssql_password_set=yes
       ;;
   esac],
  [mssql_password_set=no])

if test x$mssql_password_set = xyes; then
  AC_MSG_RESULT(['$mssql_password'])
elif test x$mssql_user = xodb_test; then
  mssql_password=odb_test
  mssql_password_set=yes
  AC_MSG_RESULT(['$mssql_password'])
else
  AC_MSG_RESULT([none])
  if test x$mssql_user_set = xyes; then
    AC_MSG_ERROR([password not specified; SQL Server requires a password (--with-mssql-password=PASS)])
  fi
fi

# Database. If not specified, use the user's default.
#
AC_MSG_CHECKING([for mssql database name])
AC_ARG_WITH(
  [mssql-db],
  [AC_HELP_STRING([--with-mssql-db=NAME], [SQL Server database name (odb_test by default). Note that all data in this database WILL BE LOST!])],
  [case $withval in
     yes)
       mssql_db=odb_test
       mssql_db_set=yes
       ;;
     no)
       mssql_db_set=no
       ;;
     *)
       mssql_db=$withval
       mssql_db_set=yes
       ;;
   esac],
  [mssql_db=odb_test
   mssql_db_set=yes])

if test x$mssql_db_set = xyes; then
  AC_MSG_RESULT(['$mssql_db'])
else
  AC_MSG_RESULT([default])
fi

# Server.
#
AC_MSG_CHECKING([for mssql instance address])
AC_ARG_WITH(
  [mssql-server],
  [AC_HELP_STRING([--with-mssql-server=ADDR], [SQL Server instance address])],
  [case $withval in
     yes | no)
       mssql_server_set=no
       ;;
     *)
       mssql_server=$withval
       mssql_server_set=yes
       ;;
   esac],
  [mssql_server_set=no])

if test x$mssql_server_set = xyes; then
  AC_MSG_RESULT(['$mssql_server'])
else
  AC_MSG_RESULT([none])
  AC_MSG_ERROR([instance address not specified; SQL Server instance address is required (--with-mssql-server=ADDR)])
fi

# Driver.
#
AC_MSG_CHECKING([for mssql native client odbc driver])
AC_ARG_WITH(
  [mssql-driver],
  [AC_HELP_STRING([--with-mssql-driver=NAME], [SQL Server Native Client ODBC driver (latest available by default)])],
  [case $withval in
     yes | no)
       mssql_driver_set=no
       ;;
     *)
       mssql_driver=$withval
       mssql_driver_set=yes
       ;;
   esac],
  [mssql_driver_set=no])

if test x$mssql_driver_set = xyes; then
  AC_MSG_RESULT(['$mssql_driver'])
else
  AC_MSG_RESULT([latest])
fi

# Create options file.
#
AC_CONFIG_COMMANDS([mssql.options],
 [
   rm -f db.options
   echo '#! /bin/sh' >db-driver

   echo 'opt=' >>db-driver

   if test x$mssql_user_set = xyes; then
     echo "--user '$mssql_user'" >>db.options
     echo 'opt="$opt -U '"$mssql_user"'"' >>db-driver

     echo "--password '$mssql_password'" >>db.options
     echo 'opt="$opt -P '"$mssql_password"'"' >>db-driver
   fi

   if test x$mssql_db_set = xyes; then
     echo "--database '$mssql_db'" >>db.options
     echo 'opt="$opt -d '"$mssql_db"'"' >>db-driver
   fi

   echo "--server '$mssql_server'" >>db.options
   echo 'opt="$opt -S '"$mssql_server"'"' >>db-driver

   if test x$mssql_driver_set = xyes; then
     echo "--driver '$mssql_driver'" >>db.options
   fi

   echo 'opt="$opt -x -r -b"' >>db-driver
   echo 'if test x$[]1 != x; then' >>db-driver
   echo "  exec $mssql_client "'$opt -i $[]1' >>db-driver
   echo "else" >>db-driver
   echo "  exec $mssql_client "'$opt' >>db-driver
   echo "fi" >>db-driver

   chmod +x db-driver
 ],
 [
  mssql_client="$mssql_client"
  mssql_user="$mssql_user"
  mssql_user_set="$mssql_user_set"
  mssql_password="$mssql_password"
  mssql_db="$mssql_db"
  mssql_db_set="$mssql_db_set"
  mssql_server="$mssql_server"
  mssql_driver="$mssql_driver"
  mssql_driver_set="$mssql_driver_set"
 ])

])dnl
