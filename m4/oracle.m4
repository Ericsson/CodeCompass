dnl file      : m4/oracle.m4
dnl copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
dnl license   : GNU GPL v2; see accompanying LICENSE file
dnl
dnl ORACLE
dnl
AC_DEFUN([ORACLE], [

# Client.
#
AC_MSG_CHECKING([for oracle client program])
AC_ARG_WITH(
  [oracle-client],
  [AC_HELP_STRING([--with-oracle-client=PATH], [Oracle client program path (sqlplus by default)])],
  [case $withval in
     yes)
       oracle_client=sqlplus
       ;;
     no)
       AC_MSG_RESULT([])
       AC_MSG_ERROR([need oracle client to run the tests])
       ;;
     *)
       oracle_client=$withval
       ;;
   esac],
  [oracle_client=sqlplus])

$oracle_client -V 2>/dev/null 1>&2

if test x"$?" = x0; then
  AC_MSG_RESULT([$oracle_client])
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([oracle client is not found; consider using --with-oracle-client=PATH])
fi

# User.
#
AC_MSG_CHECKING([for oracle database user])
AC_ARG_WITH(
  [oracle-user],
  [AC_HELP_STRING([--with-oracle-user=NAME], [Oracle database user (odb_test by default)])],
  [case $withval in
     yes)
       oracle_user=odb_test
       oracle_user_set=yes
       ;;
     no)
       oracle_user_set=no
       ;;
     *)
       oracle_user=$withval
       oracle_user_set=yes
       ;;
   esac],
  [oracle_user=odb_test
   oracle_user_set=yes])

if test x$oracle_user_set = xyes; then
  AC_MSG_RESULT(['$oracle_user'])
else
  AC_MSG_RESULT([none])
fi

# Password.
#
AC_MSG_CHECKING([for oracle database password])
AC_ARG_WITH(
  [oracle-password],
  [AC_HELP_STRING([--with-oracle-password=PASS], [Oracle database password (odb_test by default)])],
  [case $withval in
     yes)
       oracle_password=odb_test
       oracle_password_set=yes
       ;;
     no)
       oracle_password_set=no
       ;;
     *)
       oracle_password=$withval
       oracle_password_set=yes
       ;;
   esac],
  [oracle_password_set=no])

if test x$oracle_password_set = xyes; then
  AC_MSG_RESULT(['$oracle_password'])
elif test x$oracle_user = xodb_test; then
  oracle_password=odb_test
  oracle_password_set=yes
  AC_MSG_RESULT(['$oracle_password'])
elif test x$oracle_user != x/; then
  AC_MSG_RESULT([none])
  AC_MSG_ERROR([password not specified; Oracle requires a password (--with-oracle-password=PASS)])
fi

# Service name.
#
AC_MSG_CHECKING([for oracle service name])
AC_ARG_WITH(
  [oracle-service],
  [AC_HELP_STRING([--with-oracle-service=NAME], [Oracle service name (default service if left empty). Note that all data associated with this user on this service WILL BE LOST!])],
  [case $withval in
     yes)
       oracle_service=
       oracle_service_set=yes
       ;;
     no)
       oracle_service_set=no
       ;;
     *)
       oracle_service=$withval
       oracle_service_set=yes
       ;;
   esac],
  [oracle_service_set=no])

if test x$oracle_service_set = xyes; then
  AC_MSG_RESULT(['$oracle_service'])
else
  AC_MSG_RESULT([default])
fi

# Host.
#
AC_MSG_CHECKING([for oracle database host])
AC_ARG_WITH(
  [oracle-host],
  [AC_HELP_STRING([--with-oracle-host=HOST], [Oracle database host (localhost by default)])],
  [case $withval in
     yes)
       oracle_host=localhost
       oracle_host_set=yes
       ;;
     no)
       oracle_host_set=no
       ;;
     *)
       oracle_host=$withval
       oracle_host_set=yes
       ;;
   esac],
  [oracle_host_set=no])

if test x$oracle_host_set = xyes; then
  AC_MSG_RESULT(['$oracle_host'])
else
  AC_MSG_RESULT([localhost])
fi

# Port.
#
AC_MSG_CHECKING([for oracle database port])
AC_ARG_WITH(
  [oracle-port],
  [AC_HELP_STRING([--with-oracle-port=PORT], [Oracle database port (standard Oracle port by default)])],
  [case $withval in
     yes)
       oracle_port=1521
       oracle_port_set=yes
       ;;
     no)
       oracle_port_set=no
       ;;
     *)
       oracle_port=$withval
       oracle_port_set=yes
       ;;
   esac],
  [oracle_port_set=no])

if test x$oracle_port_set = xyes; then
  AC_MSG_RESULT(['$oracle_port'])
else
  AC_MSG_RESULT([default])
fi

# Create options file.
#
AC_CONFIG_COMMANDS([oracle.options],
 [
   rm -f db.options
   echo '#! /bin/sh' >db-driver

   echo 'conn_str=' >>db-driver

   if test x$oracle_user_set = xyes; then
     echo "--user '$oracle_user'" >>db.options
     echo 'conn_str="'"$oracle_user"'"' >>db-driver
   fi

   if test x$oracle_password_set = xyes; then
     echo "--password '$oracle_password'" >>db.options
     echo 'conn_str="$conn_str/'"$oracle_password"'"' >>db-driver
   fi

   if test x$oracle_host_set = xyes; then
     echo "--host '$oracle_host'" >>db.options
     echo 'conn_str="$conn_str@//'"$oracle_host"'"' >>db-driver

     if test x$oracle_port_set = xyes; then
       echo "--port '$oracle_port'" >>db.options
       echo 'conn_str="$conn_str:'"$oracle_port"'"' >>db-driver
     fi
   fi

   if test x$oracle_service != x; then
     if test x$oracle_host_set = xno; then
       echo 'conn_str="$conn_str@"' >>db-driver
     fi

     echo "--service '$oracle_service'" >>db.options
     echo 'conn_str="$conn_str/'"$oracle_service"'"' >>db-driver
   fi

   echo 'if test x$[]1 != x; then' >>db-driver
   echo "  exec $oracle_client -L -S "'$conn_str @$[]1' >>db-driver
   echo "else" >>db-driver
   echo "  exec $oracle_client -L -S "'$conn_str' >>db-driver
   echo "fi" >>db-driver

   chmod +x db-driver
 ],
 [
  oracle_client="$oracle_client"

  oracle_user="$oracle_user"
  oracle_user_set="$oracle_user_set"

  oracle_password="$oracle_password"
  oracle_password_set="$oracle_password_set"

  oracle_service="$oracle_service"
  oracle_service_set="$oracle_service_set"

  oracle_host="$oracle_host"
  oracle_host_set="$oracle_host_set"

  oracle_port="$oracle_port"
  oracle_port_set="$oracle_port_set"
 ])

])dnl
