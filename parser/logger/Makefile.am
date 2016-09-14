AM_CPPFLAGS+=-I$(top_srcdir)/parser/logger/include

bin_logtoxmldir=$(includedir)/parser/logger
bin_PROGRAMS+=bin/logtoxml
bin_logtoxml_SOURCES=\
  parser/logger/logtoxml.cpp
bin_logtoxml_CXXFLAGS=$(AM_CXXFLAGS)
bin_logtoxml_LDADD=$(AM_LDADD) -lboost_program_options 

loggerdir=$(prefix)/logger/bin
logger_PROGRAMS=logger/bin/ldlogger
logger_bin_ldlogger_SOURCES= \
  parser/logger/src/ldlogger-hooks.h \
  parser/logger/src/ldlogger-logger.c \
  parser/logger/src/ldlogger-tool.c \
  parser/logger/src/ldlogger-tool.h \
  parser/logger/src/ldlogger-tool-gcc.c \
  parser/logger/src/ldlogger-tool-javac.c \
  parser/logger/src/ldlogger-util.c \
  parser/logger/src/ldlogger-util.h
logger_bin_ldlogger_CPPFLAGS=-D_GNU_SOURCE -D__LOGGER_MAIN__
logger_bin_ldlogger_CFLAGS=$(AM_CFLAGS) -ansi -pedantic -Wall -Wextra
logger_bin_ldlogger_LDADD=
logger_bin_ldlogger_LDFLAGS=
logger_bin_ldlogger_LIBS=
logger_bin_ldlogger_LIBTOOLFLAGS=

loggerso64dir=$(prefix)/logger/lib/x86_64
loggerso64_LTLIBRARIES=logger/lib/x86_64/ldlogger.la
logger_lib_x86_64_ldlogger_la_SOURCES= \
  parser/logger/src/ldlogger-hooks.c \
  parser/logger/src/ldlogger-hooks.h \
  parser/logger/src/ldlogger-logger.c \
  parser/logger/src/ldlogger-tool.c \
  parser/logger/src/ldlogger-tool.h \
  parser/logger/src/ldlogger-tool-gcc.c \
  parser/logger/src/ldlogger-tool-javac.c \
  parser/logger/src/ldlogger-util.c \
  parser/logger/src/ldlogger-util.h

logger_lib_x86_64_ldlogger_la_LIBADD=
logger_lib_x86_64_ldlogger_la_LIBS=
logger_lib_x86_64_ldlogger_la_CPPFLAGS= \
  -D_GNU_SOURCE
logger_lib_x86_64_ldlogger_la_CFLAGS= \
  -m64 \
  -Wno-strict-aliasing \
  -fno-exceptions \
  -fPIC \
  -fomit-frame-pointer \
  -ansi \
  -Wall \
  -Wextra
logger_lib_x86_64_ldlogger_la_LDFLAGS= \
  -module \
  -avoid-version \
  -shared \
  -ldl

bin_SCRIPTS+=\
  $(top_srcdir)/parser/logger/setldlogenv.sh\
  $(top_srcdir)/parser/logger/setldlogenv.csh

EXTRA_DIST+= create_package.sh
