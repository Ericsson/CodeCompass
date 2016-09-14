dnl file      : m4/mysql.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl MYSQL
dnl
AC_DEFUN([MYSQL], [

# Client.
#
AC_MSG_CHECKING([for mysql client program])
AC_ARG_WITH(
  [mysql-client],
  [AC_HELP_STRING([--with-mysql-client=PATH], [MySQL client program path (mysql by default)])],
  [case $withval in
     yes)
       mysql_client=mysql
       ;;
     no)
       AC_MSG_RESULT([])
       AC_MSG_ERROR([need mysql client to run the tests])
       ;;
     *)
       mysql_client=$withval
       ;;
   esac],
  [mysql_client=mysql])

$mysql_client --version 2>/dev/null 1>&2

if test x"$?" = x0; then
  AC_MSG_RESULT([$mysql_client])
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([mysql client is not found; consider using --with-mysql-client=PATH])
fi

# User.
#
AC_MSG_CHECKING([for mysql database user])
AC_ARG_WITH(
  [mysql-user],
  [AC_HELP_STRING([--with-mysql-user=NAME], [MySQL database user (odb_test by default)])],
  [case $withval in
     yes)
       mysql_user=odb_test
       mysql_user_set=yes
       ;;
     no)
       mysql_user_set=no
       ;;
     *)
       mysql_user=$withval
       mysql_user_set=yes
       ;;
   esac],
  [mysql_user=odb_test
   mysql_user_set=yes])

if test x$mysql_user_set = xyes; then
  AC_MSG_RESULT(['$mysql_user'])
else
  AC_MSG_RESULT([none])
fi

# Password.
#
AC_MSG_CHECKING([for mysql database password])
AC_ARG_WITH(
  [mysql-password],
  [AC_HELP_STRING([--with-mysql-password=PASS], [MySQL database password (no password by default)])],
  [case $withval in
     yes)
       mysql_password=
       mysql_password_set=yes
       ;;
     no)
       mysql_password_set=no
       ;;
     *)
       mysql_password=$withval
       mysql_password_set=yes
       ;;
   esac],
  [mysql_password_set=no])

if test x$mysql_password_set = xyes; then
  AC_MSG_RESULT(['$mysql_password'])
else
  AC_MSG_RESULT([none])
fi

# Database name.
#
AC_MSG_CHECKING([for mysql database name])
AC_ARG_WITH(
  [mysql-db],
  [AC_HELP_STRING([--with-mysql-db=NAME], [MySQL database name (odb_test by default). Note that all data in this database WILL BE LOST!])],
  [case $withval in
     yes)
       mysql_db=odb_test
       mysql_db_set=yes
       ;;
     no)
       mysql_db_set=no
       ;;
     *)
       mysql_db=$withval
       mysql_db_set=yes
       ;;
   esac],
  [mysql_db=odb_test
   mysql_db_set=yes])

if test x$mysql_db_set = xyes; then
  AC_MSG_RESULT(['$mysql_db'])
else
  AC_MSG_RESULT([none])
fi

# Host.
#
AC_MSG_CHECKING([for mysql database host])
AC_ARG_WITH(
  [mysql-host],
  [AC_HELP_STRING([--with-mysql-host=HOST], [MySQL database host (localhost by default)])],
  [case $withval in
     yes)
       mysql_host=localhost
       mysql_host_set=yes
       ;;
     no)
       mysql_host_set=no
       ;;
     *)
       mysql_host=$withval
       mysql_host_set=yes
       ;;
   esac],
  [mysql_host_set=no])

if test x$mysql_host_set = xyes; then
  AC_MSG_RESULT(['$mysql_host'])
else
  AC_MSG_RESULT([localhost])
fi

# Port.
#
AC_MSG_CHECKING([for mysql database port])
AC_ARG_WITH(
  [mysql-port],
  [AC_HELP_STRING([--with-mysql-port=PORT], [MySQL database port (standard MySQL port by default)])],
  [case $withval in
     yes)
       mysql_port=0
       mysql_port_set=yes
       ;;
     no)
       mysql_port_set=no
       ;;
     *)
       mysql_port=$withval
       mysql_port_set=yes
       ;;
   esac],
  [mysql_port_set=no])

if test x$mysql_port_set = xyes; then
  AC_MSG_RESULT(['$mysql_port'])
else
  AC_MSG_RESULT([default])
fi

# Socket.
#
AC_MSG_CHECKING([for mysql database socket])
AC_ARG_WITH(
  [mysql-socket],
  [AC_HELP_STRING([--with-mysql-socket=SOCKET], [MySQL database socket (standard MySQL socket by default)])],
  [case $withval in
     yes)
       mysql_socket=
       mysql_socket_set=yes
       ;;
     no)
       mysql_socket_set=no
       ;;
     *)
       mysql_socket=$withval
       mysql_socket_set=yes
       ;;
   esac],
  [mysql_socket_set=no])

if test x$mysql_socket_set = xyes; then
  AC_MSG_RESULT(['$mysql_socket'])
else
  AC_MSG_RESULT([default])
fi

# Create options file.
#
AC_CONFIG_COMMANDS([mysql.options],
 [
   rm -f db.options
   echo '#! /bin/sh' >db-driver

   echo 'opt=' >>db-driver

   if test x$mysql_user_set = xyes; then
     echo "--user '$mysql_user'" >>db.options
     echo 'opt="$opt --user='"$mysql_user"'"' >>db-driver
   fi

   if test x$mysql_password_set = xyes; then
     echo "--password '$mysql_password'" >>db.options
     echo 'opt="$opt --password='"$mysql_password"'"' >>db-driver
   fi

   if test x$mysql_db_set = xyes; then
     echo "--database '$mysql_db'" >>db.options
     echo 'opt="$opt --database='"$mysql_db"'"' >>db-driver
   fi

   if test x$mysql_host_set = xyes; then
     echo "--host '$mysql_host'" >>db.options
     echo 'opt="$opt --host='"$mysql_host"'"' >>db-driver
   fi

   if test x$mysql_port_set = xyes; then
     echo "--port '$mysql_port'" >>db.options
     echo 'opt="$opt --port='"$mysql_port"'"' >>db-driver
   fi

   if test x$mysql_socket_set = xyes; then
     echo "--socket '$mysql_socket'" >>db.options
     echo 'opt="$opt --socket='"$mysql_socket"'"' >>db-driver
   fi

   echo 'if test x$[]1 != x; then' >>db-driver
   echo "  exec $mysql_client "'$opt <$[]1' >>db-driver
   echo "else" >>db-driver
   echo "  exec $mysql_client "'$opt' >>db-driver
   echo "fi" >>db-driver

   chmod +x db-driver
 ],
 [
  mysql_client="$mysql_client"

  mysql_user="$mysql_user"
  mysql_user_set="$mysql_user_set"

  mysql_password="$mysql_password"
  mysql_password_set="$mysql_password_set"

  mysql_db="$mysql_db"
  mysql_db_set="$mysql_db_set"

  mysql_host="$mysql_host"
  mysql_host_set="$mysql_host_set"

  mysql_port="$mysql_port"
  mysql_port_set="$mysql_port_set"

  mysql_socket="$mysql_socket"
  mysql_socket_set="$mysql_socket_set"
 ])

])dnl
